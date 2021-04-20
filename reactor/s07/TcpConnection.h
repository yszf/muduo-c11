#ifndef MUDUO_REACTOR_TCPCONNECTION_H
#define MUDUO_REACTOR_TCPCONNECTION_H

#include "muduo-c11/base/noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <memory>
#include <string>

namespace muduo {

    class EventLoop;
    class Socket;
    class Channel;

    class TcpConnection : noncopyable, 
                          public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(EventLoop* loop, 
                    const std::string& name,
                    int sockfd, 
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr);

        ~TcpConnection();

        EventLoop* getLoop() const {
            return loop_;
        }

        const std::string& name() const {
            return name_;
        }

        const InetAddress& localAddress() {
            return localAddr_;
        }

        const InetAddress& peerAddress() {
            return peerAddr_;
        }

        bool connected() const {
            return kConnected == state_;
        }

        void setConnectionCallback(const ConnectionCallback& cb) {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback& cb) {
            messageCallback_ = cb;
        }

        void setCloseCallback(const CloseCallback& cb) {
            closeCallback_ = cb;
        }

        void connectEstablished();

        void connectDestroyed();

    private:
        enum StateE {
            kDisconnected,
            kConnecting, 
            kConnected,
        };

        void setState(StateE s) {
            state_ = s;
        }

        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        EventLoop* loop_;
        std::string name_;
        StateE state_;
        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;
        InetAddress localAddr_;
        InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;
        Buffer inputBuffer_;

    }; // class TcpConnection

} // namesapce muduo

#endif // MUDUO_REACTOR_TCPCONNECTION_H