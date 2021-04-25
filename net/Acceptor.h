#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include "muduo-c11/base/noncopyable.h"


namespace muduo {

    namespace net {

        class EventLoop;

        class Acceptor : noncopyable {
        public:
            Acceptor(EventLoop* loop);
            ~Acceptor();

        private:
            EventLoop* loop_;

        }; // class Acceptor

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_ACCEPTOR_H