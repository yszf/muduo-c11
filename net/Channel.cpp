#include "Channel.h"
#include "EventLoop.h"

#include <assert.h>
#include <poll.h>
#include <iostream>
#include <sstream>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true), 
    tied_(false), 
    eventHanding_(false),
    addedToLoop_(false) {

}

Channel::~Channel() {
    assert(!eventHanding_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread()) {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    }
    else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    eventHanding_ = true;
    std::cout << "[Channel::handleEventWithGuard] " << reventsToString() << std::endl;
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (logHup_) {
            std::cout << "[Channel::handleEventWithGuard] fd = " << fd_ << " Channel::handle_event() POLLHUP" << std::endl;
        }
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & POLLNVAL) {
        std::cout << "[Channel::handleEventWithGuard] fd = " << fd_ << " Channel::handle_event() POLLNVAL" << std::endl;
    }

    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }

    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHanding_ = false;
}

string Channel::reventsToString() const {
    return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev) {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN) {
        oss << "IN ";
    }

    if (ev & POLLPRI) {
        oss << "PRI ";
    }

    if (ev & POLLOUT) {
        oss << "OUT ";
    }

    if (ev & POLLHUP) {
        oss << "HUP ";
    }

    if (ev & POLLRDHUP) {
        oss << "RDHUP ";
    }

    if (ev & POLLERR) {
        oss << "ERR ";
    }

    if (ev & POLLNVAL) {
        oss << "NVAL ";
    }

    return oss.str();
}