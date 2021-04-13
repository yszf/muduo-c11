#ifndef MUDUO_REACTOR_EVENTLOOP_H
#define MUDUO_REACTOR_EVENTLOOP_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Thread.h"
#include "TimerId.h"
#include "Callbacks.h"
#include <vector>
#include <memory>

namespace muduo {

    class Channel;
    class Poller;
    class TimerQueue;
    class EventLoop : noncopyable {
    public:
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

        bool isInLoopThread() {
            return threadId_ == CurrentThread::tid();
        }

        Timestamp pollReturnTime() const {
            return pollReturnTime_;
        }

        // runs callback at time
        TimerId runAt(const Timestamp& time, const TimerCallback& cb);

        // runs callback after delay seconds
        TimerId runAfter(double delay, const TimerCallback& cb);

        // runs callback every interval seconds
        TimerId runEvery(double interval, const TimerCallback& cb);

    private:
        void abortNotInLoopThread();

        typedef std::vector<Channel*> ChannelList;

        bool looping_; // atomic
        bool quit_; // atomic
        const pid_t threadId_;
        Timestamp pollReturnTime_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        ChannelList activeChannels_;

    };

} // namespace muduo


#endif // MUDUO_REACTOR_EVENTLOOP_H