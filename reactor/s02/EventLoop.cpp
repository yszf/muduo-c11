#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"

#include <iostream>
#include <assert.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() 
    : looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    pollReturnTime_(Timestamp::invalid()),
    poller_(new Poller(this)),
    timerQueue_(new TimerQueue(this)) {
    std::cout << "[EventLoop::EventLoop] trace: EventLoop " << this << " Created in thread " << threadId_ << std::endl;

    if (t_loopInThisThread) {
        std::cout << "[EventLoop::EventLoop] fatal: Another EventLoop " << t_loopInThisThread << " exsits in this thread " << threadId_ << std::endl;
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
    assertInLoopThread();
    assert(!looping_);
    looping_ = true;
    quit_ = false;

    while(!quit) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
    }

    std::cout << "[EventLoop::loop] trace: EventLoop " << this << " stop looping" << std::endl;
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "[EventLoop::abortNotInLoopThread] fatal: EventLoop " << this << " was created in threadId_ " << threadId_ << ", current thread id = " << CurrentThread::tid() << std::endl;
    assert(false);
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}