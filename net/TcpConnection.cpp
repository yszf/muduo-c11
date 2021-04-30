#include "TcpConnection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketsOps.h"

#include <iostream>

using namespace muduo;
using namespace muduo::net;

void muduo::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
    std::cout << "defaultConnectionCallback: local -> peer" << conn->localAddress().toIpPort() << " -> " << conn->peerAddress().toIpPort() << " is " << (conn->connected() ? "UP" : "DOWN");
}

void muduo::net::defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop, const string& name,int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr) 
    : loop_(loop),
    name_(name), 
    state_(kConnecting), 
    reading_(true), 
    socket_(new Socket(sockfd)), 
    channel_(new Channel(loop, sockfd)), 
    localAddr_(localAddr), 
    peerAddr_(peerAddr), 
    highWaterMark_(64 * 1024 * 1024) {
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    std::cout << "TcpConnection::ctor[" << name_ << "] at " << this << " fd = " << sockfd << std::endl;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    std::cout << "TcpConnection::dtor[" << name_ << "] at " << this << " fd = "<< channel_->fd() << " state = " << stateToString() << std::endl;
    assert(kDisconnected == state_);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const {
    return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const void* data, int len) {
    send(string(static_cast<const char*>(data), len));
}

void TcpConnection::send(const string& message) {
    if (kConnected == state_) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        }
        else {
            void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, message));
        }
    }
}

void TcpConnection::send(Buffer* buf) {
    if (kConnected == state_) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else {
            void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const string& message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (kDisconnected == state_) {
        std::cout << "[TcpConnection::sendInLoop] disconnected, give up writing" << std::endl;
        return;
    }

    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (0 == remaining && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                std::cout << "[TcpConnection::sendInLoop] syserr" << std::endl;

                if (EPIPE == errno || ECONNRESET == errno) {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }

        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);

        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (kConnected == state_) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose() {
    if (kConnected == state_ || kDisconnecting == state_) {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
    if (kConnected == state_ || kDisconnecting == state_) {
        setState(kDisconnecting);
        // not forceCloseInLoop to avoid race condition
        // loop_->runAfter(seconds, makeWeakCallback(shared_from_this(), &TcpConnection::forceClose));
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (kConnected == state_ || kDisconnecting == state_) {
        handleClose();
    }
}

const char* TcpConnection::stateToString() const {
    switch (state_) {
    case kDisconnected:
        return "kDisconnected";
    case kConnecting:
        return "kConnecting";
    case kConnected:
        return "kConnected";
    case kDisconnecting:
        return "kDisconnecting";
    default:
        return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}
