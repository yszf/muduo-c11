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

        TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);

        void reset(Timestamp now);

    private:
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;

        void handleRead();
        std::vector<Entry> getExpired(Timestamp now);

        EventLoop* loop_;
        TimerList timers_;
        Channel timerChannel_;
        int timerfd_;
        Timestamp expiration_;
        const double interval_;
        bool repeat_;

    }; // class TimerQueue

} // namespace muduo

#endif // MUDUO_REACTOR_TIMERQUEUE_H