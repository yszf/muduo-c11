#ifndef MUDUO_REACTOR_TCPSERVER_H
#define MUDUO_REACTOR_TCPSERVER_H

#include "muduo-c11/base/noncopyable.h"
#include "Callbacks.h"
#include "TcpConnection.h"

#include <string>
#include <memory>
#include <map>

namespace muduo {

    class EventLoop;
    class Acceptor;
    class InetAddress;
    class EventLoopThreadPool;

    class TcpServer : noncopyable {
    public:
        TcpServer(EventLoop* loop, const InetAddress& listenAddr);
        ~TcpServer();

        void setThreadNum(int numThreads);

        void start();

        void setConnectionCallback(const ConnectionCallback& cb) {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback& cb) {
            messageCallback_ = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
            writeCompleteCallback_ = cb;
        }

    private:
        // not thread safe, but in loop
        void newConnection(int sockfd, const InetAddress& peerAddr);
        // thread safe
        void removeConnection(const TcpConnectionPtr& conn);
        // not thread safe, but in loop
        void removeConnectionInLoop(const TcpConnectionPtr& conn);

        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop* loop_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;
        std::unique_ptr<EventLoopThreadPool> threadPool_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool started_;
        int nextConnId_; // always in loop thread
        ConnectionMap connections_;

    }; // class TcpServer

} // namespace muduo

#endif // MUDUO_REACTOR_TCPSERVER_H