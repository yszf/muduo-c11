#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"

#include <iostream>


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

}

TcpConnection::~TcpConnection() {
    std::cout << "debug: TcpConnection::dtor[" << name_ << "] at " << this << ", fd = " << channel_->fd() << std::endl;
}