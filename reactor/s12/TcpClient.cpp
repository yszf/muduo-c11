#include "TcpClient.h"
#include "EventLoop.h"
#include "Connector.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "SocketsOps.h"

#include <iostream>

using namespace muduo;

namespace muduo {

    namespace detail {
        
        void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
            loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
        }

        void removeConnector(const ConnectorPtr& connector) {

        }
    }

}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr) 
    : loop_(loop), 
    connector_(new Connector(loop, serverAddr)), 
    retry_(false), 
    connect_(false), 
    nextConnId_(1) {

    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));

    std::cout << "info: TcpClient::TcpClient[" << this << "] - connector " << connector_.get() << std::endl;
}

TcpClient::~TcpClient() {
    std::cout << "info: TcpClient::~TcpClient[" << this << "] - connector " << connector_.get() << std::endl;

    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    }

    if (conn) {
        // FIXME: not 100% safe, if we are in different thread
        CloseCallback cb = std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    }
    else {
        connector_->stop();
        loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    // FIXME: check state
    std::cout << "info: TcpClient::connect[" << this << "] - connecting to " << connector_->serverAddress().toHostPort() << std::endl; 

    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;

    {
        MutexLockGuard lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    string connName = buf;

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe

    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }

    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

    if (retry_ && connect_) {
        std::cout << "info: TcpClient::connect[" << this << "] - Reconnecting to " << connector_->serverAddress().toHostPort() << std::endl;
        connector_->restart();
    }
}

