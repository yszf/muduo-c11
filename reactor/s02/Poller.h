#ifndef MUDUO_REACTOR_POLLER_H
#define MUDUO_REACTOR_POLLER_H

#include "muduo-c11/base/noncopyable.h"
#include "EventLoop.h"

#include <vector>
#include <map>

struct pollfd;

namespace muduo {

    class Channel;
    // IO Multiplexing with poll
    class Poller : noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(EventLoop* loop);
        ~Poller();

        // Polls the I/O events 轮询
        // Must be called in the loop thread
        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        // Changes the interested I/O events.
        // Must be called in the loop thread.
        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            loop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel*> ChannelMap;

        EventLoop* loop_;
        PollFdList pollfds_;
        ChannelMap channels_;

    }; // class Poller

} // namespace muduo


#endif // MUDUO_REACTOR_POLLER_H