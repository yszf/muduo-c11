#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Types.h"

#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <memory>

struct tcp_info;

namespace muduo {

    namespace net {

        class EventLoop;
        class Socket;
        class Channel;

        class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
        public:
            TcpConnection(EventLoop* loop, const string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
            
            ~TcpConnection();

            EventLoop* getLoop() const {
                return loop_;
            }

            const string& name() const {
                return name_;
            }

            const InetAddress& localAddress() const {
                return localAddr_;
            }

            const InetAddress& peerAddress() const {
                return peerAddr_;
            }

            bool connected() const {
                return kConnected == state_;
            }

            bool disconnected() const {
                return kDisconnected == state_;
            }

            bool getTcpInfo(struct tcp_info*) const;
            string getTcpInfoString() const;

            void send(const void* message, int len);
            void send(const string& message);
            void send(Buffer* message);

            void shutdown();
            void forceClose();
            void forceCloseWithDelay(double seconds);
            void setTcpNoDelay(bool on);

            void startRead();
            void stopRead();
            bool isReading() const {
                return reading_;
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

            void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) {
                highWaterMarkCallback_ = cb;
            }

            void setCloseCallback(const CloseCallback& cb) {
                closeCallback_ = cb;
            }

            Buffer* inputBuffer() {
                return &inputBuffer_;
            }

            Buffer* outputBuffer() {
                return &outputBuffer_;
            }

            void connectEstablished();

            void connectDestroyed();
            
        private:
            enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

            void handleRead(Timestamp receiveTime);
            void handleWrite();
            void handleClose();
            void handleError();

            void sendInLoop(const string& message);
            void sendInLoop(const void* message, size_t len);
            void shutdownInLoop();
            void forceCloseInLoop();
            void setState(StateE s) { state_ = s; }
            const char* stateToString() const;
            void startReadInLoop();
            void stopReadInLoop();

            EventLoop* loop_;
            const string name_;
            StateE state_;
            bool reading_;
            std::unique_ptr<Socket> socket_;
            std::unique_ptr<Channel> channel_;
            const InetAddress localAddr_;
            const InetAddress peerAddr_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            HighWaterMarkCallback highWaterMarkCallback_;
            CloseCallback closeCallback_;
            size_t highWaterMark_;
            Buffer inputBuffer_;
            Buffer outputBuffer_;

        };

        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    } // namespace net

} // namespace muduo


#endif // MUDUO_NET_TCPCONNECTION_H