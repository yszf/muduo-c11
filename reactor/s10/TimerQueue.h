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

    private:
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;

        void addTimerInLoop(Timer* timer);
        void handleRead();
        std::vector<Entry> getExpired(Timestamp now);
        void reset(const std::vector<Entry>& expired, Timestamp now);

        bool insert(Timer* timer);

        EventLoop* loop_;
        const int timerfd_;
        Channel timerChannel_;
        TimerList timers_;

    }; // class TimerQueue

} // namespace muduo

#endif // MUDUO_REACTOR_TIMERQUEUE_H