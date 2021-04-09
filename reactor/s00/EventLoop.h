#ifndef MUDUO_BASE_EVENTLOOP_H
#define MUDUO_BASE_EVENTLOOP_H

#include "muduo-c11/base/Thread.h"
#include "muduo-c11/base/noncopyable.h"

namespace muduo {

    class EventLoop : public noncopyable {
    public:
        EventLoop();
        ~EventLoop();

        void loop();

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const {
            return threadId_ == CurrentThread::tid();
        }
    private:
        void abortNotInLoopThread();

        bool looping_; // atomic
        const pid_t threadId_;

    }; // class EventLoop

} // namespace muduo


#endif // MUDUO_BASE_EVENTLOOP_H