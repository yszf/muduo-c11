#ifndef MUDUO_REACTOR_TIMERQUEUE_H
#define MUDUO_REACTOR_TIMERQUEUE_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

#include <set>
#include <vector>


namespace muduo {

    class EventLoop;
    class Timer;
    class TimerId;
    class TimerQueue : noncopyable {
    public:
        TimerQueue(EventLoop* loop);
        ~TimerQueue();

        // Must be thread safe. Usually be called from other threads.
        TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);

    private:
        // FIXME: use unique_ptr<Timer> instead of raw pointers. 
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;

        // called when timerfd alarms
        void handleRead();

        // move out all expired timers
        std::vector<Entry> getExpired(Timestamp now);

        // reset repeat timer from expired timers
        void reset(const std::vector<Entry>& expired, Timestamp now);

        EventLoop* loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        // Timer list sorted by expiration
        TimerList timers_;

    }; // class TimerQueue


} // namespace muduo

#endif // MUDUO_REACTOR_TIMERQUEUE_H
