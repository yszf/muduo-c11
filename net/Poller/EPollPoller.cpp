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
    std::cout << "[EPollPoller::EPollPoller] epollfd = " << epollfd_ << std::endl;
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    std::cout << "[EPollPoller::poll] fd total count " << channels_.size() << std::endl;
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
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

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    assert(implicit_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        activeChannels->emplace_back(channel);
    }
}

void EPollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    std::cout << "[EPollPoller::updateChannel] fd = " << channel->fd() << " events = " << channel->events() << " index = " << index << std::endl;

    if (kNew == index || kDeleted == index) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (kNew == index) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else { // kDeleted == index
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(kAdded == index);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    std::cout << "[EPollPoller::removeChannel] fd = " << fd << std::endl;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(kAdded == index || kDeleted == index);
    size_t n = channels_.erase(fd);
    assert(1 == n);
    if (kAdded == index) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    muduo::memZero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    std::cout << "[EPollPoller::update] epoll_ctl op = " << operationToString(operation) << " fd = " << fd << " event = { " << channel->eventsToString() << " }" << std::endl;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        std::cout << "[EPollPoller::update] syserr: epoll_ctl op = " << operationToString(operation) << " fd = " << fd << std::endl;
    }
}

const char* EPollPoller::operationToString(int op) {
    switch (op) {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}