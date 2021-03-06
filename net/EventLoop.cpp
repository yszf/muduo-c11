#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include "TimerQueue.h"
#include "SocketsOps.h"

#include <sys/eventfd.h>
#include <signal.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

namespace {
    __thread EventLoop* t_loopInThisThread = nullptr;

    const int kPollTimeMs = 10000;

    int createEventfd() {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0) {
            std::cout << "[createEventfd] syserr: Failed in create eventfd" << std::endl;
            abort();
        }
        return evtfd;
    }

    class IgnoreSigPipe {
    public:
        IgnoreSigPipe() {
            ::signal(SIGPIPE, SIG_IGN);
        }
    };

    IgnoreSigPipe initObj;

} // namespace

EventLoop::EventLoop() 
    : looping_(false), 
    quit_(false), 
    eventHandling_(false), 
    callingPendingFunctors_(false), 
    iteration_(0),
    threadId_(CurrentThread::tid()), 
    poller_(Poller::newDefaultPoller(this)), 
    timerQueue_(new TimerQueue(this)), 
    wakeupFd_(createEventfd()), 
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentActiveChannel_(nullptr) {
    std::cout << "EventLoop " << this << " created in thread " << threadId_ << std::endl;
    if (t_loopInThisThread) {
        std::cout << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << std::endl;
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
    std::cout << "EventLoop " << this << "of thread " << threadId_ << " destructs in thread " << CurrentThread::tid() << std::endl;
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    std::cout << "EventLoop " << this << " start looping" << std::endl;

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, & activeChannels_);
        ++iteration_;

        printActiveChannels();

        eventHandling_ = true;
        for (Channel* channel : activeChannels_) {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }

    std::cout << "EventLoop " << this << " stop looping" << std::endl;
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

size_t EventLoop::queueSize() const {
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_) {
        ChannelList::iterator it = std::find(activeChannels_.begin(), activeChannels_.end(), channel);
        assert(currentActiveChannel_ == channel || it == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "[EventLoop::abortNotInLoopThread] EventLoop " << this << " was created in threadId_ = " << threadId_ << " , current thread id = " << CurrentThread::tid() << std::endl;
    abort();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        std::cout << "[EventLoop::wakeup] writes " << n << " bytes instead of 8" << std::endl;
        assert(false);
    }
}

void EventLoop::handleRead() {
    uint64_t one;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        std::cout << "[EventLoop::handleRead] reads " << n << " bytes instead of 8" << std::endl;
        assert(false);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
    for (const Channel* channel : activeChannels_) {
        std::cout << "{" << channel->reventsToString() << "} "; 
    }
    std::cout << std::endl;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}