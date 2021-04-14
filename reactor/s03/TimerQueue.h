#ifndef MUDUO_REACTOR_TIMERQUEUE_H
#define MUDUO_REACTOR_TIMERQUEUE_H

#include "muduo-c11/base/noncopyable.h"
#include "Callbacks.h"
#include "Channel.h"

#include <vector>
#include <set>

namespace muduo {

    class EventLoop;
    class Timer;
    class TimerId;
    class Timestamp;
    class TimerQueue : noncopyable {
    public:
        TimerQueue(EventLoop* loop);
        ~TimerQueue();

        TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);

    private:
        // FIXME: use unique_ptr<Timer> instead of raw pointer
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;

        void addTimerInLoop(Timer* timer);
        // called when timerfd alarms
        void handleRead();
        // move out all expired timers
        std::vector<Entry> getExpired(Timestamp now);
        // reset repeat timer
        void reset(const std::vector<Entry>& expired, Timestamp now);

        bool insert(Timer* timer);

        EventLoop* loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        // Timer list sorted by expiration
        TimerList timers_;

    }; // class TimerQueue

} // namespace muduo

#endif // MUDUO_REACTOR_TIMERQUEUE_H