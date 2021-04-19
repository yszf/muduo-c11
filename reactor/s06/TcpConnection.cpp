#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <iostream>
#include <unistd.h>


using namespace muduo;


TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr) 
    : loop_(loop), 
    name_(name), 
    state_(kConnecting), 
    socket_(new Socket(sockfd)), 
    channel_(new Channel(loop, sockfd)), 
    localAddr_(localAddr), 
    peerAddr_(peerAddr) {

    std::cout << "debug: TcpConnection::ctor[" << name_ << "] at " << this << ", fd = " << sockfd << std::endl;

    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    std::cout << "debug: TcpConnection::dtor[" << name_ << "] at " << this << ", fd = " << channel_->fd() << std::endl;
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(kConnecting == state_); 
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(kConnected == state_);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead() {
    char buf[65536];
    ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
    if (n > 0) {
        messageCallback_(shared_from_this(), buf, n);
    }
    else if (0 == n) {
        handleClose();
    }
    else {
        handleError();
    }

    // FIXME: close connection if n == 0s
}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    std::cout << "trace: TcpConnection::handleClose state = " << state_ << std::endl;
    assert(kConnected == state_);
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    std::cout << "error: TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << std::endl;
}