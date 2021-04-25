#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include "muduo-c11/base/noncopyable.h"

#include <functional>

namespace muduo {

    namespace net {

        class EventLoop;
        class Timestamp;

        class Channel : noncopyable {
        public:
            typedef std::function<void()> EventCallback;
            typedef std::function<void(Timestamp)> ReadEventCallback;

            Channel(EventLoop* loop, int fd);
            ~Channel();

            void handleEvent(Timestamp receiveTime);

            void setReadCallback(ReadEventCallback cb) {
                readCallback_ = std::move(cb);
            }

            void setWriteCallback(EventCallback cb) {
                writeCallback_ = std::move(cb);
            }

            void setCloseCallback(EventCallback cb) {
                closeCallback_ = std::move(cb);
            }

            void setErrorCallback(EventCallback cb) {
                errorCallback_ = std::move(cb);
            }

            int fd() const {
                return fd_;
            }

            int events() const {
                return events_;
            }

            void set_revents(int revt) {
                revents_ = revt;
            }

            int revents() const {
                return revents_;
            }

            bool isNoneEvent() const {
                return events_ == kNoneEvent;
            }

            void enableReading() {
                events_ |= kReadEvent; 
                update();
            }

            void disableReading() {
                events_ &= ~kReadEvent;
                update();
            }

            void enableWriting() {
                events_ |= kWriteEvent;
                update();
            }

            void disableWriting() {
                events_ &= ~kWriteEvent;
                update();
            }

            void disableAll() {
                events_ = kNoneEvent;
                update();
            }

            bool isReading() const {
                return events_ & kReadEvent;
            }

            bool isWriting() const {
                return events_ & kWriteEvent;
            }

            // for Poller
            int index() {
                return index_;
            }

            void set_index(int idx) {
                index_ = idx;
            }

            EventLoop* ownerLoop() {
                return loop_;
            }

            void remove();

        private:
            void update();

            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;

            EventLoop* loop_;
            const int fd_;
            int events_;
            int revents_;
            int index_; // used by Poller

            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback closeCallback_;
            EventCallback errorCallback_;

        }; // class Channel

    } // namespace net

} // namespace muduo


#endif // MUDUO_NET_CHANNEL_H