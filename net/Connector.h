#ifndef MUDUO_NET_CONNECTOR_H
#define MUDUO_NET_CONNECTOR_H

#include "muduo-c11/base/noncopyable.h"


namespace muduo {

    namespace net {

        class EventLoop;

        class Connector : noncopyable {
        public:
            Connector(EventLoop* loop);
            ~Connector();
        private:
            EventLoop* loop_;
        }; // class Connector

    }

}


#endif // MUDUO_NET_CONNECTOR_H