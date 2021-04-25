#ifndef MUDUO_REACTOR_TCPCLIENT_H
#define MUDUO_REACTOR_TCPCLIENT_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Mutex.h"
#include "Callbacks.h"
#include "TcpConnection.h"

#include <memory>

namespace muduo {

    class EventLoop;
    class Connector;
    typedef std::shared_ptr<Connector> ConnectorPtr;
    class InetAddress;

    class TcpClient : noncopyable {
    public:
        TcpClient(EventLoop* loop, const InetAddress& serverAddr);
        
        ~TcpClient();

        void connect();

        void disconnect();

        void stop();

        TcpConnectionPtr connection() const {
            MutexLockGuard lock(mutex_);
            return connection_;
        }

        EventLoop* getLoop() const {
            return loop_;
        }

        bool retry() const {
            return retry_;
        }

        void enableRetry() {
            retry_ = true;
        }

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
        void newConnection(int sockfd);
        // not thread safe, but in loop
        void removeConnection(const TcpConnectionPtr& conn); 

        EventLoop* loop_;
        ConnectorPtr connector_; // avoid revealing Connector
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool retry_;  // atomic
        bool connect_; // atomic
        int nextConnId_; // always in loop thread
        mutable MutexLock mutex_;
        TcpConnectionPtr connection_;
    }; // class TcpClient

} // namespace muduo

#endif // MUDUO_REACTOR_TCPCLIENT_H