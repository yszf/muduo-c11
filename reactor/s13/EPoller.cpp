#include "EPoller.h"
#include "Channel.h"
#include "muduo-c11/base/Timestamp.h"

#include <poll.h>
#include <sys/epoll.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

using namespace muduo;

namespace {
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}


EPoller::EPoller(EventLoop* loop) 
    : loop_(loop), 
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)), 
    events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        std::cout << "EPoller::EPoller" << std::endl;
        assert(false);
    }
}

EPoller::~EPoller() {
    ::close(epollfd_);
}

Timestamp EPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);

    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        std::cout << "[EPoller::poll] "<< numEvents << " events happended" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (0 == numEvents) {
        std::cout << "[EPoller::poll] nothing happended" << std::endl;
    }
    else {
        std::cout << "error: EPoller::poll()" << std::endl;
        assert(false);
    }
    return now;
}

void EPoller::fillActiveChannels(int numEvents, ChannelList*  activeChannels) const {
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

void EPoller::updateChannel(Channel* channel) {
    assertInLoopThread();
    std::cout << "[EPoller::updateChannel] fd = " << channel->fd() << " events = " << channel->events() << std::endl;

    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else { // index == kDeleted
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
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel* channel) {
    assertInLoopThread();
    int fd = channel->fd();
    std::cout << "[EPoller::removeChannel] fd = " << fd << std::endl;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    assert(1 == n);

    if (kAdded == index) {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->set_index(kNew);
}

void EPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        std::cout << "err: epoll_ctl op = " << operation << " fd = " << fd << std::endl;
        assert(false);
    }
}