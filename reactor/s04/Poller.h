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

        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            loop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList* activeChannels);

        typedef std::map<int, Channel*> ChannelMap;
        typedef std::vector<struct pollfd> PollfdList;

        EventLoop* loop_;
        ChannelMap channels_;
        PollfdList pollfds_;
    };

} // namespace muduo

#endif // MUDUO_REACTOR_POLLER_H