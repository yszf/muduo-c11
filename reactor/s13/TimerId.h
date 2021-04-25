#ifndef MUDUO_REACTOR_TIMERID_H
#define MUDUO_REACTOR_TIMERID_H

#include "muduo-c11/base/copyable.h"
#include "Timer.h"


namespace muduo {

    class TimerId : public copyable {
    public:
        TimerId(Timer* timer = nullptr, int64_t seq = 0) : 
            timer_(timer),
            sequence_(seq) {

        }

        ~TimerId() {

        }

        friend class TimerQueue;

    private:
        Timer* timer_;
        int64_t sequence_;
        
    }; // class TimerId

} // namespace muduo

#endif // MUDUO_REACTOR_TIMERID_H