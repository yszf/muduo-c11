#include "EventLoop.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "TimerId.h"
#include "muduo-c11/base/Timestamp.h"

#include <unistd.h>
#include <iostream>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() 
    : looping_(false), 
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(new Poller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(0),
    wakeupChannel_(new Channel(this, wakeupFd_)) {
    std::cout << "EventLoop " << this << " creates in thread " << threadId_ << threadId_ << std::endl;
    if (t_loopInThisThread) {
        std::cout << "[EventLoop::EventLoop] fatal: Another loop " << t_loopInThisThread << " was created in this thread " << threadId_ <<  std::endl;
        assert(false);
    }
    else {
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
        doPendingFunctors();
    }

    std::cout << "[EventLoop::loop] trace: EventLoop " << this << " stop looping" << std::endl;
    looping_ = false;
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }

    callingPendingFunctors_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
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

void EventLoop::runInLoop(const Functor& cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t data = 1;
    ssize_t n = ::write(wakeupFd_, &data, sizeof data);
    if (n != sizeof data) {
        std::cout << "[EventLoop::wakeup] error: EventLoop::wakeup() writes " << n << "bytes instead of 8" << std::endl;
        assert(false);
    }
}

void EventLoop::handleRead() {
    uint64_t data;
    ssize_t n = ::read(wakeupFd_, &data, sizeof data);
    if (n != sizeof data) {
        std::cout << "[EventLoop::handleRead] error: EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
        assert(false);
    }
}