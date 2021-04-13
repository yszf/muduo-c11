#ifndef MUDUO_REACTOR_CHANNEL_H
#define MUDUO_REACTOR_CHANNEL_H

#include "muduo-c11/base/noncopyable.h"

#include <functional>


namespace muduo {

    class EventLoop;
    class Channel : noncopyable {
    public:
        typedef std::function<void()> EventCallback;

        // the file descriptor coult be a socket, an eventfd, a timerfd, or a signalfd
        Channel(EventLoop* loop, int fd);
        ~Channel();

        void handEvent();

        void setReadCallback(const EventCallback& cb) {
            readCallback_ = cb;
        }

        void setWriteCallback(const EventCallback& cb) {
            writeCallback_ = cb;
        }

        void setErrorCallback(const EventCallback& cb) {
            errorCallback_ = cb;
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

        bool isNoneEvent() {
            return kNoneEvent == events_;
        }

        void enableReading() {
            events_ |= kReadEvent;
            update();
        }

        int index() {
            return index_;
        }

        void set_index(int idx) {
            index_ = idx;
        }

        EventLoop* ownerLoop() {
            return loop_;
        }

    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop* loop_;
        int fd_;
        int events_;
        int revents_;
        int index_; // used by Poller

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;

    }; // class Channel


} // namespace muduo


#endif // MUDUO_REACTOR_CHANNEL_H