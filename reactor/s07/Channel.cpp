#include "Channel.h"
#include "EventLoop.h"

#include <poll.h>
#include <iostream>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop), 
    fd_(fd), 
    events_(0),
    revents_(0),
    index_(-1), 
    eventHandling_(false) {

}

Channel::~Channel() {
    assert(!eventHandling_);
}

void Channel::handleEvent(Timestamp receiveTime) {
    eventHandling_ = true;
    if (revents_ & POLLNVAL) {
        std::cout << "[Channel::handleEvent] warn: POLLNVAL" << std::endl;
    }

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        std::cout << "[Channel::handleEvent] warn: POLLHUP" << std::endl;
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }

    if (revents_ & (POLLOUT)) {
        if (writeCallback_) writeCallback_();
    }

    eventHandling_ = false;
}

void Channel::update() {
    loop_->updateChannel(this);
}