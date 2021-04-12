#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <sys/timerfd.h>
#include <iostream>
#include <unistd.h>

namespace muduo {

    namespace detail {

        int createTimerfd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

            if (timerfd < 0) {
                std::cout << "sysfatal: Failed in timerfd_create" << std::endl;
            }
            return timerfd;
        }

    } // namespace detail

} // namespace muduo

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop* loop) 
    : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_), 
    timers_() {

    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);

    for (TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval) {
    Timer* timer = new Timer(cb, when, interval);
    loop_->assertInLoopThread();
  //  bool earliestChanged = insert(timer);


}