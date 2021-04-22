#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include "SocketsOps.h"

#include <iostream>

using namespace muduo;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop), 
    serverAddr_(serverAddr), 
    connect_(false), 
    state_(kDisconnected), 
    retryDelayMs_(kInitRetryDelayMs) {
    std::cout << "debug: Connector ctor[" << this << "]" << std::endl;
}

Connector::~Connector() {
    std::cout << "debug: Connector dtor[" << this << "]" << std::endl;
    loop_->cancel(timerId_);
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(kDisconnected == state_);
    if (connect_) {
        connect();
    }
    else {
        std::cout << "[Connector::startInLoop] debug: do not connect" << std::endl;
    }
}

void Connector::connect() {
    int sockfd = sockets::createNonblockingOrDie();
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());
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
        std::cout << "syserr: connect error in Connector::startInLoop, savedErrno =  " << savedErrno << std::endl;
        sockets::close(sockfd);
        break;
    default:
        std::cout << "syserr: unexpected error in Connector::startInLoop, savedErrno =  " << savedErrno << std::endl;
        sockets::close(sockfd);
        break;
    }
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
}

void Connector::stop() {
    connect_ = false;
    loop_->cancel(timerId_);
}

void Connector::connecting(int sockfd) {
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(std::bind(&Connector::handleError, this)); // FIXME: unsafe

    channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}

void Connector::handleWrite() {
    std::cout << "trace: Connector::handleWrite, state =  " << state_ << std::endl;

    if (kConnecting == state_) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err) {
            std::cout << "warn: Connector::handleWrite - SO_ERROR = " << err << std::endl;
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd)) {
            std::cout << "warn: Connector::handleWrite - Self connect" << std::endl;
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
    std::cout << "[Connector::handleError] error: Connector::handleError" << std::endl;
    assert(kConnecting == state_);

    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    std::cout << "[Connector::handleError] SO_ERROR = " << err << std::endl;
    retry(sockfd);
}

void Connector::retry(int sockfd) {
    sockets::close(sockfd);
    setState(kDisconnected);

    if (connect_) {
        std::cout << "[Connector::retry] info: Retry connecting to " << serverAddr_.toHostPort() << " in " << retryDelayMs_ << " millliseconds" << std::endl;

        timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0,std::bind(&Connector::startInLoop, this));

        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else {
        std::cout << "[Connector::retry] debug: do not connect" << std::endl;
    }
}