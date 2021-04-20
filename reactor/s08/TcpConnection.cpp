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

    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    std::cout << "debug: TcpConnection::dtor[" << name_ << "] at " << this << ", fd = " << channel_->fd() << std::endl;
}

void TcpConnection::send(const std::string& message) {
    if (kConnected == state_) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        }
        else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message.data(), message.size());

        if (nwrote >= 0) {
            if (implicit_cast<size_t>(nwrote) < message.size()) {
                std::cout << "[TcpConnection::sendInLoop] trace: I am going to write more data" << std::endl;
            }
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                std::cout << "syserr: TcpConnection::sendInLoop" << std::endl;
                assert(false);
            }
        }
    }

    assert(nwrote >= 0);
    if (implicit_cast<size_t>(nwrote) < message.size()) {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
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

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(kConnecting == state_); 
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(kConnected == state_ || kDisconnecting == state_);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    // char buf[65536];
    // ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
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
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());

        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (kDisconnecting == state_) {
                    shutdownInLoop();
                }
            }
            else {
                std::cout << "[TcpConnection::handleWrite] trace: I am going to write more data" << std::endl;
            }
        }
        else {
            std::cout << "syserr: TcpConnection::handleWrite" << std::endl;
            assert(false);
        }
    }
    else {
        std::cout << "[TcpConnection::handleWrite] trace: Connection is down, no more writing" << std::endl;
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    std::cout << "trace: TcpConnection::handleClose state = " << state_ << std::endl;
    assert(kConnected == state_ || kDisconnecting == state_);
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    std::cout << "error: TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << std::endl;
}