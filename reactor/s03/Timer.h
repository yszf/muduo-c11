#ifndef MUDUO_REACTOR_TIMER_H
#define MUDUO_REACTOR_TIMER_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Timestamp.h"
#include "Callbacks.h"

namespace muduo {

    class Timer : noncopyable {
    public:
        Timer(const TimerCallback& cb, Timestamp when, double interval) 
            : callback_(cb), 
            expiration_(when), 
            interval_(interval), 
            repeat_(interval > 0.0) {

        }

        void run() const {
            callback_();
        }

        Timestamp expiration() const {
            return expiration_;
        }

        bool repeat() const {
            return repeat_;
        }

        void restart(Timestamp now);
    
    private:
        const TimerCallback callback_;
        Timestamp expiration_;
        const double interval_;
        const bool repeat_;

    }; // class Timer

} // namespace muduo

#endif // MUDUO_REACTOR_TIMER_H