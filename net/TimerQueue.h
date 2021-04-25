#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include "muduo-c11/base/noncopyable.h"
#include "Callbacks.h"
#include "Channel.h"

#include <set>

namespace muduo {

    namespace net {

        class EventLoop;
        class Timestamp;
        class Timer;
        class TimerId;

        class TimerQueue : noncopyable {
        public:
            explicit TimerQueue(EventLoop* loop);
            ~TimerQueue();

            TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

            void cancel(TimerId timerId);

        private:
            typedef std::pair<Timestamp, Timer*> Entry;
            typedef std::set<Entry> TimerList;
            typedef std::pair<Timer*, int64_t> ActiveTimer;
            typedef std::set<ActiveTimer> ActiveTimerSet;

            EventLoop* loop_;
            const int timerfd_;
            Channel timerfdChannel_;
            TimerList timers_; // Timer list sorted by expiration
            // for cancel
            ActiveTimerSet activeTimers_;
            bool callingExpiredTimers_; // atomic
            ActiveTimerSet cancelingTimers_;

        }; // class TimerQueue

    } // namespace muduo

} // namespace net

#endif // MUDUO_NET_TIMERQUEUE_H