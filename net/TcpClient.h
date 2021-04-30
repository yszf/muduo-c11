#ifndef MUDUO_NET_TCPCLIENT_H
#define MUDUO_NET_TCPCLIENT_H

#include "muduo-c11/base/Mutex.h"
#include "TcpConnection.h"

#include <memory>

namespace muduo {

    namespace net {

        class Connector;
        typedef std::shared_ptr<Connector> ConnectorPtr;

        class TcpClient : noncopyable {
        public:
            TcpClient(EventLoop* loop, const InetAddress& serverAddr, const string& name);
            ~TcpClient();

            void connect();
            void disconnect();
            void stop();

            TcpConnectionPtr connection() const {
                MutexLockGuard lock(mutex_);
                return connection_;
            }

            EventLoop* getLoop() const { return loop_; }
            bool retry() const { return retry_; }
            void enableRetry() {
                retry_ = true;
            }

            const string& name() const {
                return name_;
            }

            void setConnectionCallback(ConnectionCallback cb) {
                connectionCallback_ = std::move(cb);
            }

            void setMessageCallback(MessageCallback cb) {
                messageCallback_ = std::move(cb);
            }

            void setWriteCompleteCallback(WriteCompleteCallback cb) {
                writeCompleteCallback_ = std::move(cb);
            }
        private:
            void newConnection(int sockfd);

            void removeConnection(const TcpConnectionPtr& conn);

            EventLoop* loop_;
            ConnectorPtr connector_;
            const string name_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            bool retry_; // atomic
            bool connect_; // atomic
            int nextConnId_;
            mutable MutexLock mutex_;
            TcpConnectionPtr connection_;
        }; // class TcpClient

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_TCPCLIENT_H