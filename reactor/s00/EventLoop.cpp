#include "muduo-c11/reactor/s00/EventLoop.h"
#include "muduo-c11/base/Logging.h"
#include <poll.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop() 
    : looping_(false), 
    threadId_(CurrentThread::tid()) {
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread) {
        LOG_FATAL << "EventLoop created " << this << " in thread " << threadId_;
    }
    else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    ::poll(nullptr, 0, 10 * 1000);

    LOG_TRACE << "EventLoop " << this << " stop looping";

    looping_ = false;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this << " was created in threadId_ = " << threadId_ << ", current thread id = " << CurrentThread::tid();
}