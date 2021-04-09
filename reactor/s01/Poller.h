#ifndef MUDUO_REACTOR_POLLER_H
#define MUDUO_REACTOR_POLLER_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "EventLoop.h"

#include <vector>
#include <map>

struct pollfd;

namespace muduo {

    class Channel;
    class Poller : noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(EventLoop* loop);
        ~Poller();

        // Polls the I/O events
        // Must be called in the loop thread
        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        // Changes the interested I/O events
        // Must be called in the loop thread
        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            ownerLoop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel*> ChannelMap;

        EventLoop* ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };

} // namespace muduo

#endif // MUDUO_REACTOR_POLLER_H