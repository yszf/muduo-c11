#ifndef MUDUO_REACTOR_EVENTLOOPTHREAD_H
#define MUDUO_REACTOR_EVNETLOOPTHREAD_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Thread.h"

namespace muduo {

    class EventLoop;
    class EventLoopThread : noncopyable {
    public:
        EventLoopThread();
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFunc();

        EventLoop* loop_;
        Thread thread_;
        bool exiting_;
        MutexLock mutex_;
        Condition cond_;

    }; // class EventLoopThread

} // namespace muduo

#endif // MUDUO_REACTOR_EVENTLOOPTHREAD_H