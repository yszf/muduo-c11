#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <iostream>

namespace muduo {

    namespace detail {

        int createTimerfd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            if (timerfd < 0 ) {
                std::cout << "[createTimerfd] sysfatal: Failed in timerfd_create" << std::endl;
                assert(false);
            }
            return timerfd;
        }

        struct timespec howMuchTimeFromNow(Timestamp when) {
            int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();

            if (microseconds < 100) {
                microseconds = 100;
            }

            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<long>(microseconds % Timestamp::kMicroSecondsPerSecond) * 1000;

            return ts;
        } 

        void resetTimerfd(int timerfd, Timestamp expiration) {
            struct itimerspec newValue;
            struct itimerspec oldValue;
            bzero(&newValue, sizeof newValue);
            bzero(&oldValue, sizeof oldValue);
            newValue.it_value = howMuchTimeFromNow(expiration);
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
            if (ret) {
                std::cout << "[resetTimerfd] syserr: timerfd_settime()" << std::endl;
                assert(false);
            }
        }

        void readTimerfd(int timerfd, Timestamp now) {
            uint64_t data;
            ssize_t n = ::read(timerfd, &data, sizeof data);
            if (n != sizeof data) {
                std::cout << "[readTimerfd] error: TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;
                assert(false);
            }
        }

    }

}

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
    timerfd_(createTimerfd()),
    timerChannel_(loop, timerfd_),
    timers_() {

    timerChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerChannel_.enableReading();
} 

TimerQueue::~TimerQueue() {
    ::close(timerfd_);

    for (TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval) {
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer);
 }

 void TimerQueue::addTimerInLoop(Timer* timer) {
     loop_->assertInLoopThread();
     bool earliestChanged = insert(timer);

     if (earliestChanged) {
         resetTimerfd(timerfd_, timer->expiration());
     }
 }

 bool TimerQueue::insert(Timer* timer) {
     bool earliestChanged = false;
     Timestamp when = timer->expiration();
     TimerList::iterator it = timers_.begin();
     if (it == timers_.end() || when < it->first) {
         earliestChanged = true;
     }

     std::pair<TimerList::iterator, bool> result = timers_.insert(std::make_pair(when, timer));
     assert(result.second);
     return earliestChanged;
 }

 void TimerQueue::handleRead() {
     loop_->assertInLoopThread();
     Timestamp now(Timestamp::now());
     readTimerfd(timerfd_, now);

     std::vector<Entry> expired = getExpired(now);

     for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); ++it) {
         it->second->run();
     }
     reset(expired, now);
 }


std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
    for (std::vector<Entry>::const_iterator it = expired.begin(); it != expired.end(); ++it) {
        if (it->second->repeat()) {
            it->second->restart(now);
            insert(it->second);
        }
        else {
            delete it->second;
        }
    }

    Timestamp nextExpire;
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}