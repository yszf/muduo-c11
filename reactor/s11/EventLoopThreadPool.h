#ifndef MUDUO_REACTOR_EVENTLOOPTHREADPOOL_H
#define MUDUO_REACTOR_EVENTLOOPTHREADPOOL_H

#include "muduo-c11/base/noncopyable.h"

#include <memory>
#include <vector>

namespace muduo {

    class EventLoop;
    class EventLoopThread;

    class EventLoopThreadPool : noncopyable {
    public:
        EventLoopThreadPool(EventLoop* baseLoop);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads) {
            numThreads_ = numThreads;
        }

        void start();

        EventLoop* getNextLoop();

    private:
        EventLoop* baseLoop_;
        bool started_;
        int numThreads_;
        int next_; // always in loop thread
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop*> loops_;

    };

} // namespace muduo

#endif // MUDUO_REACTOR_EVNETLOOPTHREAD_H