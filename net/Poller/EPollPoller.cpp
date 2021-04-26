#include "EPollPoller.h"
#include "muduo-c11/net/Channel.h"
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop* loop) 
    : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        std::cout << "[EPollPoller::EPollPoller] sysfatal" << std::endl;
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

muduo::Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    std::cout << "[EPollPoller::poll] fd total count " << channels_.size() << std::endl;
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;
    muduo::Timestamp now(muduo::Timestamp::now());
    if (numEvents > 0) {
        std::cout << "[EPollPoller::poll] " << numEvents << " events happended" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (0 == numEvents) {
        std::cout << "[EPollPoller::poll] nothing happended" << std::endl;
    }
    else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            std::cout << "[EPollPoller::poll] syserr" << std::endl;
            assert(false);
        }
    }
    return now;
}