#include "muduo-c11/net/Poller.h"
#include "muduo-c11/net/Poller/PollPoller.h"
#include "muduo-c11/net/Poller/EPollPoller.h"

#include <stdlib.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return new PollPoller(loop);
    }
    else {
        return new EPollPoller(loop);
    }
}