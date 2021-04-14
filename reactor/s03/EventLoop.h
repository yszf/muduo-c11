#ifndef MUDUO_REACTOR_EVENTLOOP_H
#define MUDUO_REACTOR_EVENTLOOP_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Timestamp.h"
#include "muduo-c11/base/Mutex.h"
#include "Callbacks.h"
#include "TimerId.h"

#include <vector>
#include <memory>

namespace muduo {

    class Poller;
    class Channel;
    class TimerQueue;
    class EventLoop : noncopyable {
    public:
        typedef std::function<void()> Functor;

        EventLoop();
        ~EventLoop();

        void loop();

        void quit();

        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const {
            return threadId_ == CurrentThread::tid();
        }

        TimerId runAt(const Timestamp& time, const TimerCallback& cb);

        TimerId runAfter(double delay, const TimerCallback& cb);

        TimerId runEvery(double interval, const TimerCallback& cb);

        // Time when poll returns, usually means data arrival
        Timestamp pollReturnTime() const{
            return pollReturnTime_;
        }

        // Safe to call from other threads
        // Runs callback immediately in the loop thread.
        // It wakes up the loop, and run the cb.
        void runInLoop(const Functor& cb);

        // Safe to call from other threads
        // Queues callback in the loop thread.
        // Runs after finish pooling.
        void queueInLoop(const Functor& cb);

        void wakeup();

    private:
        void handleRead(); // waked up
        void doPendingFunctors();
        void abortNotInLoopThread();

        typedef std::vector<Channel*> ChannelList;

        bool looping_; // atomic
        bool quit_; // atomic
        bool callingPendingFunctors_; // atomic
        const pid_t threadId_;
        Timestamp pollReturnTime_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        ChannelList activeChannels_;
        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;
        MutexLock mutex_;
        std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_

    }; // class EventLoop

} // namespace muduo

#endif // MUDUO_REACTOR_EVENTLOOP_H