#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include "muduo-c11/base/noncopyable.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"

namespace muduo {

    namespace net {

        class EventLoop;

        class Acceptor : noncopyable {
        public:
            typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

            Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);

            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback& cb) {
                newConnectionCallback_ = cb;
            }

            bool listenning() const {
                return listening_;
            }

            void listen();
            
        private:
            void handleRead();

            EventLoop* loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
            bool listening_;
            int idleFd_;

        }; // class Acceptor

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_ACCEPTOR_H