#ifndef MUDUO_NET_POLLER_POLLPOLLER_H
#define MUDUO_NET_POLLER_POLLPOLLER_H

#include "muduo-c11/net/Poller.h"
#include <vector>

struct pollfd;

namespace muduo {

    namespace net {

        class PollPoller : public Poller {
        public:
            PollPoller(EventLoop* loop);
            ~PollPoller() override;

            muduo::Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

            void updateChannel(Channel* channel) override;

            void removeChannel(Channel* channel) override;

        private:
            void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

            typedef std::vector<struct pollfd> PollFdList;
            PollFdList pollfds_;
        
        }; // class PollPoller

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_POLLER_POLLPOLLER_H