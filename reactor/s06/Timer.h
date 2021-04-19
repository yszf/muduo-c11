#ifndef MUDUO_REACTOR_TIMER_H
#define MUDUO_REACTOR_TIMER_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "Callbacks.h"

namespace muduo {

    class Timer : noncopyable {
    public:
        Timer(const TimerCallback& cb, Timestamp when, double interval);
        ~Timer();

        void run() {
            callback_();
        }

        void restart(Timestamp now);

        Timestamp expiration() const {
            return expiration_;
        }

        bool repeat() const {
            return repeat_;
        }

    private:
        const TimerCallback callback_;
        Timestamp expiration_;
        double interval_;
        bool repeat_;

    }; // class Timer

} // namespace muduo


#endif // MUDUO_REACTOR_TIMER_H