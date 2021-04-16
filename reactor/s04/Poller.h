#ifndef MUDUO_REACTOR_POLLER_H
#define MUDUO_REACTOR_POLLER_H

#include "muduo-c11/base/noncopyable.h"
#include "EventLoop.h"

#include <vector>
#include <map>

struct pollfd;

namespace muduo {

    class Channel;
    class Timestamp;
    class Poller : noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(EventLoop* loop);
        ~Poller();

        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            loop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

        typedef std::vector<struct pollfd> PollfdList;
        typedef std::map<int, Channel*> ChannelMap;

        EventLoop* loop_;
        PollfdList pollfds_;
        ChannelMap channels_;

    }; // class Poller

} // namespace muduo

#endif // MUDUO_REACTOR_POLLER_H