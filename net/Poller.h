#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "EventLoop.h"

#include <map>
#include <vector>

namespace muduo {

    namespace net {

        class Channel;

        class Poller : noncopyable {
        public:
            typedef std::vector<Channel*> ChannelList;

            Poller(EventLoop* loop);
            virtual ~Poller();

            virtual muduo::Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

            virtual void updateChannel(Channel* channel) = 0;

            virtual void removeChannel(Channel* channel) = 0;

            virtual bool hasChannel(Channel* channel) const;

            static Poller* newDefaultPoller(EventLoop* loop);

            void assertInLoopThread() const {
                loop_->assertInLoopThread();
            }

        protected:
            typedef std::map<int, Channel*> ChannelMap;
            ChannelMap channels_;

        private:
            EventLoop* loop_;

        }; // class Poller

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_POLLER_H