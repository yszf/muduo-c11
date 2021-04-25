#ifndef MUDUO_REACTOR_EVENTLOOP_H
#define MUDUO_REACTOR_EVENTLOOP_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Mutex.h"
#include "muduo-c11/base/Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"

#include <vector>
#include <memory>
#include <functional>

namespace muduo {

    class EPoller;
    class Channel;
    class TimerQueue;

    class EventLoop : noncopyable {
    public:
        typedef std::function<void()> Functor;

        EventLoop();
        ~EventLoop();

        void loop();

        void quit();

        Timestamp pollReturnTime() const {
            return pollReturnTime_;
        }

        void runInLoop(const Functor& cb);

        void queueInLoop(const Functor& cb);

        TimerId runAt(const Timestamp& time, const TimerCallback& cb);

        TimerId runAfter(double delay, const TimerCallback& cb);

        TimerId runEvery(double interval, const TimerCallback& cb);

        void cancel(TimerId timerId);

        void wakeup();

        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);

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
        void handleRead();
        void doPendingFunctors();


        typedef std::vector<Channel*> ChannelList;

        bool looping_;
        bool quit_;
        bool callingPendingFunctors_;
        const pid_t threadId_;
        Timestamp pollReturnTime_;
        std::unique_ptr<EPoller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;
        ChannelList activeChannels_;
        MutexLock mutex_;
        std::vector<Functor> pendingFunctors_;

    }; // class EventLoop

} // namespace muduo

#endif // MUDUO_REACTOR_EVENTLOOP_H