#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "muduo-c11/base/Mutex.h"
#include "TimerId.h"
#include "Callbacks.h"

#include <sched.h> // pid_t
#include <atomic>
#include <memory> // unique_ptr
#include <functional>
#include <vector>

namespace muduo {

    namespace net {

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

            Timestamp pollReturnTime() const { 
                return pollReturnTime_;
            }

            int64_t iteration() const {
                return iteration_;
            }

            void runInLoop(Functor cb);

            void queueInLoop(Functor cb);

            size_t queueSize() const;

            TimerId runAt(Timestamp time, TimerCallback cb);

            TimerId runAfter(double delay, TimerCallback cb);

            TimerId runEvery(double interval, TimerCallback cb);

            void cancel(TimerId timerId);

            void wakeup();

            void updateChannel(Channel* channel);
            
            void removeChannel(Channel* channel);

            bool hasChannel(Channel* channel);

            pid_t threadId() const {
                return threadId_;
            }

            void assertInLoopThread() {
                if (!isInLoopThread()) {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const {
                return threadId_ == CurrentThread::tid();
            }

            bool callingPendingFunctors() const {
                return callingPendingFunctors_;
            }

            bool eventHanding() const {
                return eventHandling_;
            }

            static EventLoop* getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();
            void handleRead(); // waked up
            void doPendingFunctors();

            void printActiveChannels() const; // DEBUG

            typedef std::vector<Channel*> ChannelList;

            bool looping_; // atomic
            std::atomic<bool> quit_;
            bool eventHandling_; // atomic
            bool callingPendingFunctors_; // atomic
            int64_t iteration_;
            const pid_t threadId_;
            Timestamp pollReturnTime_;
            std::unique_ptr<Poller> poller_;
            std::unique_ptr<TimerQueue> timerQueue_;

            int wakeupFd_;
            std::unique_ptr<Channel> wakeupChannel_;

            ChannelList activeChannels_;
            Channel* currentActiveChannel_;

            mutable MutexLock mutex_;
            std::vector<Functor> pendingFunctors_;

        }; // class EventLoop

    } // namespace net

}

#endif // MUDUO_NET_EVENT_H