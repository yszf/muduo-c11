#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Types.h"
#include "muduo-c11/base/Atomic.h"
#include "TcpConnection.h"

#include <memory>
#include <map>

namespace muduo {

    namespace net {

        class EventLoop;
        class Acceptor;
        class EventLoopThreadPool;

        class TcpServer : noncopyable {
        public:
            typedef std::function<void(EventLoop*)> ThreadInitCallback;

            enum Option {
                kNoReusePort,
                kReusePort,
            };

            TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, Option option= kNoReusePort);

            ~TcpServer();

            const string& ipPort() const {
                return ipPort_;
            }

            const string& name() const {
                return name_;
            }

            EventLoop* getLoop() const {
                return loop_;
            }

            void setThreadNum(int numThreads);

            void setThreadInitCallback(const ThreadInitCallback& cb) {
                threadInitCallback_ = cb;
            }

            std::shared_ptr<EventLoopThreadPool> threadPool() {
                return threadPool_;
            }

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
            void newConnection(int sockfd, const InetAddress& peerAddr);

            void removeConnection(const TcpConnectionPtr& conn);

            void removeConnectionInLoop(const TcpConnectionPtr& conn);

            typedef std::map<string, TcpConnectionPtr> ConnectionMap;

            EventLoop* loop_;
            const string ipPort_;
            const string name_;
            std::unique_ptr<Acceptor> acceptor_;
            std::shared_ptr<EventLoopThreadPool> threadPool_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            ThreadInitCallback threadInitCallback_;
            AtomicInt32 started_;

            int nextConnId_;
            ConnectionMap connections_;

        }; // class TcpServer


    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_TCPSERVER_H