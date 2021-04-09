#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"

#include <iostream>
#include <assert.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() : 
    looping_(false), 
    quit_(false),
    threadId_(CurrentThread::tid()),
    poller_(new Poller(this)) {
    std::cout << "trace:[EventLoop::EventLoop] EventLoop created " << this << " in thread " << threadId_ << std::endl;
    
    if (t_loopInThisThread) {
        std::cout << "fatal:[EventLoop::EventLoop] Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << std::endl;
        assert(false);
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
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
    }

    std::cout << "trace:[EventLoop::loop] EventLoop " << this << " stop looping" << std::endl;
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // wakeup();
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "fatal:[EventLoop::abortNotInLoopThread] EventLoop " << this << "was created in threadId_ = " << threadId_ << ", current thread id = " << CurrentThread::tid() << std::endl;
    assert(false); 
}