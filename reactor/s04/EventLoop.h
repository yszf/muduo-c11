#ifndef MUDUO_REACTOR_EVETNLOOP_H
#define MUDUO_REACTOR_EVENTLOOP_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Mutex.h"
#include "Channel.h"
#include "Callbacks.h"

#include <vector>
#include <memory>
#include <functional>

namespace muduo {

    class Poller;
    class Channel;
    class TimerQueue;
    class TimerId;
    class Timestamp;
    class EventLoop : noncopyable {
    public:
        typedef std::function<void()> Functor;

        EventLoop();
        ~EventLoop();

        void loop();

        void updateChannel(Channel* channel);

        TimerId runAt(const Timestamp& time, const TimerCallback& cb);

        TimerId runAfter(double delay, const TimerCallback& cb);

        TimerId runEvery(double interval, const TimerCallback& cb);

        void runInLoop();

        void queueInLoop();

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() {
            return threadId_ == CurrentThread::tid();
        }
    private:
        void handleRead();
        void abortNotInLoopThread();
        void callPendingFunctors();

        typedef std::vector<Channel*> ChannelList;

        bool looping_;
        bool quit_;
        bool callPendingFunctor_;
        pid_t threadId_;
        std::vector<Functor> functors_;
        ChannelList activeChannels_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        Channel wakeupChannel_;
        int wakeupFd_;
        MutexLock mutex_;

    }; // class EventLoop

} // namespace muduo

#endif // MUDUO_REACTOR_EVENTLOOP_H