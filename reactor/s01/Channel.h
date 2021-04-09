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

        void setReadCallback(const EventCallback& cb) {
            readCallback_ = cb;
        }

        void setWrateCallback(const EventCallback& cb) {
            writeCallback_ = cb;
        }

        void setErrorCallback(const EventCallback& cb) {
            errorCallback_ = cb;
        }

        int fd() const { return fd_; }

        int events() const { return events_; }

        void set_revents(int revt) {
            revents_ = revt;
        }

        bool isNoneEvent() const { return events_ == kNoneEvent; }

        void enalbleReading() {
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

        void disableAll() {
            events_ = kNoneEvent;
            update();
        }

        // for Poller
        int index() { return index_; }

        void set_index(int idx) { index_ = idx; }

        EventLoop* ownerLoop() { return ownerLoop_; }

    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop*      ownerLoop_;
        const int       fd_; // 对应的文件描述符
        int             events_; // 关注的事件
        int             revents_; // 就是的事件
        int             index_; // used by Poller

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
    };

} // namespace muduo 

#endif // MUDUO_REACTOR_CHANNEL_H