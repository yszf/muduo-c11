#include "muduo-c11/reactor/s00/EventLoop.h"
#include <iostream>
#include <poll.h>
#include <assert.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop() 
    : looping_(false), 
    threadId_(CurrentThread::tid()) {
    std::cout << "info: EventLoop created " << this << " in thread " << threadId_ << std::endl;
    if (t_loopInThisThread) {
        std::cout << "warn: EventLoop created " << this << " in thread " << threadId_ << std::endl;
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

    ::poll(nullptr, 0, 500 * 1000);

    std::cout << "info: EventLoop " << this << " stop looping" << std::endl;

    looping_ = false;
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "error: EventLoop::abortNotInLoopThread - EventLoop " << this << " was created in threadId_ = " << threadId_ << ", current thread id = " << CurrentThread::tid() << std::endl;
    assert(false);
}