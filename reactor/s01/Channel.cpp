#include "Channel.h"
#include "EventLoop.h"

#include <poll.h>
#include <iostream>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) : 
    ownerLoop_(loop),
    fd_(fd),
    events_(kNoneEvent),
    revents_(kNoneEvent),
    index_(-1) {}

Channel::~Channel() {}

void Channel::update() {
    ownerLoop_->updateChannel(this);
}

void Channel::handleEvent() {
    if (revents_ & POLLNVAL) {
        std::cout << "warn:[Channel::handleEvent] Channel::handleEvent() POLLNVAL" << std::endl;
    }
    
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_();
    }

    if (revents_ & (POLLOUT)) {
        if (writeCallback_) writeCallback_();
    }
}