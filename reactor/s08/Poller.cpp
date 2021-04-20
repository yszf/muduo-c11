#include "Poller.h"
#include "Channel.h"
#include "muduo-c11/base/Timestamp.h"

#include <poll.h>
#include <iostream>


using namespace muduo;


Poller::Poller(EventLoop* loop) 
    : loop_(loop) {

}

Poller::~Poller() {

}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    assertInLoopThread();
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        std::cout << "[Poller::poll] trace: " << numEvents << " events happen at " << now.toString().c_str() << std::endl;
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (0 == numEvents) {
        std::cout << "[Poller::poll] nothing happen" << std::endl;
    }
    else {
        std::cout << "[Poller::poll] error: poll()" << std::endl;
        assert(false);
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (PollfdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if (pfd->revents > 0) {
            std::cout << "[Poller::fillActiveChannels] revents: fd = " << pfd->fd << std::endl;
            --numEvents;
            ChannelMap::const_iterator it = channels_.find(pfd->fd);
            assert(it != channels_.end());
            Channel* channel = it->second;
            assert(pfd->fd == channel->fd());
            channel->set_revents(pfd->revents);
            activeChannels->emplace_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel) {
    assertInLoopThread();
    std::cout << "[Poller::updateChannel] trace: fd = " << channel->fd() << " events = " << channel->events() << std::endl; 
    if (channel->index() < 0) {
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
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void Poller::removeChannel(Channel* channel) {
    assertInLoopThread();
    std::cout << "[Poller::removeChannel] trace: fd = " << channel->fd() << std::endl;
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    
    const struct pollfd& pfd = pollfds_[idx];
    (void) pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());

    size_t n = channels_.erase(channel->fd());
    assert(1 == n); (void) n;

    if (implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    }
    else {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }

}