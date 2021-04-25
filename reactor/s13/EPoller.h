#ifndef MUDUO_REACTOR_EPOLLER_H
#define MUDUO_REACTOR_EPOLLER_H


#include "muduo-c11/base/noncopyable.h"
#include "EventLoop.h"

#include <vector>
#include <map>

struct epoll_event;

namespace muduo {

    class Channel;
    class Timestamp;

    class EPoller : noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;

        EPoller(EventLoop* loop);
        ~EPoller();

        // Polls the I/O events
        // Must be called in the loop thread
        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        // Changes the interested I/O events
        // Must be called in the loop thread
        void updateChannel(Channel* channel);

        // Remove the channel, when it destructs
        // Must be called in the loop thread
        void removeChannel(Channel* channel);

        void assertInLoopThread() {
            loop_->assertInLoopThread();
        }

    private:
        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

        void update(int operation, Channel* channel);

        typedef std::vector<struct epoll_event> EventList;
        typedef std::map<int, Channel*> ChannelMap;

        EventLoop* loop_;
        int epollfd_;
        EventList events_;
        ChannelMap channels_;

    };

} // namespace muduo

#endif // MUDUO_REACTOR_REACTOR_H