#ifndef MUDUO_REACTOR_POLLER_H
#define MUDUO_REACTOR_POLLER_H

#include "muduo-c11/base/noncopyable.h"

#include <vector>
#include <map>

struct pollfd;

namespace muduo {

    class EventLoop;
    class Channel;
    class Timestamp;
    class Poller : noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(EventLoop* loop);
        ~Poller();

        // Polls the I/O events
        // mush be called in the loop thread
        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        // must be called in the loop thread
        void updateChannel(Channel* channel);

        void assertInLoopThread();

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