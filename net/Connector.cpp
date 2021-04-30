#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include "SocketsOps.h"

#include <iostream>

using namespace muduo;
using namespace muduo::net;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr) 
    : loop_(loop), 
    serverAddr_(serverAddr), 
    connect_(false), 
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDelayMs) {
    
}

Connector::~Connector() {
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(kDisconnected == state_);
    if (connect_) {
        connect();
    }
    else {
        std::cout << "[Connector::startInLoop] do not connect" << std::endl;
    }
}

void Connector::stop() {
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop() {
    loop_->assertInLoopThread();
    if (kConnecting == state_) {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect() {
    int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());

    std::cout << "[Connector::connect] start connect " << serverAddr_.toIpPort() << " sockfd = " << sockfd << std::endl;
    
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
    int savedErrno = (0 == ret) ? 0 : errno;
    switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        std::cout << "[Connector::connect] error " << savedErrno << std::endl;
        sockets::close(sockfd);
        break;

    default:
        std::cout << "[Connector::connect] Unexpected error in Connector::connect " << savedErrno << std::endl;
        sockets::close(sockfd);
        break;
    }
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd) {
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));
    channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}

void Connector::handleWrite() {
    if (kConnecting == state_) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err) {
            std::cout << "[Connector::handleWrite] SO_ERROR = " << err << std::endl;
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd)) {
            std::cout << "[Connector::handleWrite] self connect" << std::endl;
            retry(sockfd);
        }
        else {
            setState(kConnected);
            if (connect_) {
                newConnectionCallback_(sockfd);
            }
            else {
                sockets::close(sockfd);
            }
        }
    }
    else {
        assert(kDisconnected == state_);
    }
}

void Connector::handleError() {
    if (kConnecting == state_) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        std::cout << "[Connector::handleError] SO_ERROR = " << err << std::endl;
        retry(sockfd);
    }
}

void Connector::retry(int sockfd) {
    sockets::close(sockfd);
    setState(kDisconnected);
    if (connect_) {
        std::cout << "[Connector::retry] retry connecting to " << serverAddr_.toIpPort() << " in " << retryDelayMs_ << " milliseconds." << std::endl;

        loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else {
        std::cout << "[Connector::retry] do not connect" << std::endl;
    }
}