#include "Poller.h"
#include "Channel.h"

#include <poll.h>
#include <iostream>
#include <assert.h>

using namespace muduo;


Poller::Poller(EventLoop* loop) : 
    ownerLoop_(loop) {}

Poller::~Poller() {}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    // pollfds_ shouldn't change
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        std::cout << "trace:[Poller::poll] " << numEvents << " events happended" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (0 == numEvents) {
        std::cout << "trace:[Poller::poll] nothing happended" << std::endl;
    }
    else {
        std::cout << "SysError:[Poller::poll] ::poll" << std::endl;
    }

    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
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

void Poller::updateChannel(Channel* channel) {
    assertInLoopThread();
    std::cout << "trace:[Poller::updateChannel] fd = " << channel->fd() << " events = " << channel->events() << std::endl;

    if (channel->index() < 0) {
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
        assert(pfd.fd == channel->fd() || -1 == pfd.fd);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // ignore this pollfd
            pfd.fd = -1;
        }
    }
}