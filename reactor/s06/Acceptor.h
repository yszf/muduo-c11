#ifndef MUDUO_REACTOR_ACCEPTOR_H
#define MUDUO_REACTOR_ACCEPTOR_H

#include "muduo-c11/base/noncopyable.h"

#include "Channel.h"
#include "Socket.h"

#include <functional>

namespace muduo {

    class EventLoop;
    class InetAddress;
    class Acceptor : noncopyable {
    public:
        typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

        Acceptor(EventLoop* loop, const InetAddress& listenAddr);

        void setNewConnectionCallback(const NewConnectionCallback& cb) {
            newConnectionCallback_ = cb;
        }

        bool listenning() const {
            return listenning_;
        }

        void listen();

    private:
        void handleRead();

        EventLoop* loop_;
        Socket  acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listenning_;
        
    }; // class Acceptor


} // namespace muduo

#endif // MUDUO_REACTOR_ACCEPTOR_H