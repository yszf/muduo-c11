#ifndef MUDUO_REACTOR_CHANNEL_H
#define MUDUO_REACTOR_CHANNEL_H

#include "muduo-c11/base/noncopyable.h"

#include <functional>

namespace muduo {

    class EventLoop;
    class Channel : noncopyable {
    public:
        typedef std::function<void()> EventCallback;

        Channel(EventLoop* loop, int fd);
        ~Channel();

        void handleEvent();

        int fd() const {
            return fd_;
        }

        int events() const {
            return events_;
        }

        void set_revents(int revt) {
            revents_ = revt;
        }

        int index() {
            return index_;
        }

        void set_index(int idx) {
            index_ = idx;
        }

        void set_readCallback(const EventCallback& cb) {
            readCallback_ = cb;
        }

        void set_writeCallback(const EventCallback& cb) {
            writeCallback_ = cb;
        }

        void set_errorCallback(const EventCallback& cb) {
            errorCallback_ = cb;
        }

        void enableReading() {
            events_ |= kReadEvent;
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

        void disableAllEvent() {
            events_ = kNoneEvent;
            update();
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
        int index_;

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;

    }; // class Channel

} // namespace muduo

#endif // MUDUO_REACTOR_CHANNEL_H
