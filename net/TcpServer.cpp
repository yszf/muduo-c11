#include "TcpServer.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"

#include <memory>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, Option option) 
    : loop_(loop),
    ipPort_(listenAddr.toIpPort()),
    name_(name),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1) {
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    std::cout << "TcpServer::~TcpServer [" << name_ << "] destructing" << std::endl;

    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}