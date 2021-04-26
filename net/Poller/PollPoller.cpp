#include "PollPoller.h"
#include "muduo-c11/net/Channel.h"
#include "muduo-c11/net/EventLoop.h"

#include <poll.h>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

PollPoller::PollPoller(EventLoop* loop)
    : Poller(loop) {

}

PollPoller::~PollPoller() = default;

muduo::Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    muduo::Timestamp now(muduo::Timestamp::now());
    if (numEvents > 0) {
        std::cout << "[PollPoller::poll] " << numEvents << " events happended" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (0 == numEvents){
      std::cout << "[PollPoller::poll] nothing happended" << std::endl;
    }
    else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            std::cout << "[PollPoller::poll] syserr" << std::endl;
            assert(false);
        }
    }
    return now;
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->emplace_back(channel);
        }
    }
}

void PollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    std::cout << "[PollPoller::updateChannel] fd = " << channel->fd() << " events = " << channel->events() << std::endl;
    if (channel->events() < 0) {
        // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.emplace_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == channel->fd() - 1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;

        if (channel->isNoneEvent()) {
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    std::cout << "[PollPoller::removeChannel] fd = " << channel->fd() << std::endl;
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(1 == n);
    if (implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    }
    else {
        int fdAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (fdAtEnd < 0) {
            fdAtEnd = -fdAtEnd - 1;
        }
        channels_[fdAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}