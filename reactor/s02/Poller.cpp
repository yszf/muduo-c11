#include "Poller.h"
#include "Channel.h"

#include <poll.h>
#include <iostream>

using namespace muduo;


Poller::Poller(EventLoop* loop) 
    : loop_(loop) {}

Poller::~Poller() {}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);

    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        std::cout << "[Poller::poll] trace: " << numEvents << " events happended" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (0 == numEvents) {
        std::cout << "[Poller::poll] trace: nothing happended" << std::endl;
    }
    else {
        std::cout << "[Poller::poll] syserr: poll()" << std::endl;
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
    std::cout << "[Poller::updateChannel] trace: fd = " << channel->fd() << ", events = " << channel->events() << std::endl;

    if (channel->index() < 0) {
        // add a new channel
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
        // update existing channel
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channel == channels_[channel->fd()]);

        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || -1 == channel->fd());
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // ignore this pollfd
            pfd.fd = -1;
        }
    }
}