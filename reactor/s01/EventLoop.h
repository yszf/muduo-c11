#ifndef MUDUO_REACTOR_EVENTLOOP_H
#define MUDUO_REACTOR_EVENTLOOP_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Thread.h"
#include "muduo-c11/base/CurrentThread.h"

#include <vector>
#include <memory>

namespace muduo {

    class Poller;
    class Channel;

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

        bool isInLoopThread() const { 
            return threadId_ == CurrentThread::tid(); 
        }
    private:
        void abortNotInLoopThread();

        typedef std::vector<Channel*> ChannelList;

        bool looping_; // atomic
        bool quit_;  // atomic
        const pid_t threadId_;
        std::unique_ptr<Poller> poller_;
        ChannelList activeChannels_;
    };

}// namespace muduo


#endif // MUDUO_REACTOR_EVENTLOOP_H