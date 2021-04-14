#include "EventLoop.h"
#include "TimerQueue.h"
#include "Channel.h"
#include "Poller.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        std::cout << "[createEventfd] syserr: Failed in eventfd" << std::endl;
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(new Poller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)) {

    std::cout << "[EventLoop::EventLoop] trace: EventLoop " << this << " created in thread " << threadId_ << std::endl;
    if (t_loopInThisThread) {
        std::cout << "[EventLoop::EventLoop] fatal: Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << std::endl;
        assert(false);
    } 
    else {
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
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

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handEvent();
        }

        doPendingFunctors();
    }

    std::cout << "[EventLoop::loop] trace: EventLoop " << this << " stop looping" << std::endl;
    looping_ = false;
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
    uint64_t one = 1;
    std::cout << "[EventLoop::wakeup] trace: wakeup" << std::endl;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof one) {
        std::cout << "[EventLoop::wakeup] error: EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
        assert(false);
    }
}

void EventLoop::handleRead() {
    uint64_t one;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof one) {
        std::cout << "[EventLoop::handleRead] error: EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
        assert(false);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    {
        MutexLockGuard lock_(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "[EventLoop::abortNotInLoopThread] fatal: EventLoop " << this << " was created in threadId_" << threadId_ << ", current thread id = " << CurrentThread::tid() << std::endl;
    assert(false);
}