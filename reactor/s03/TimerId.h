#ifndef MUDUO_REACTOR_TIMERID_H
#define MUDUO_REACTOR_TIMERID_H

#include "muduo-c11/base/copyable.h"

namespace muduo {
    
    class Timer;
    class TimerId : public copyable {
    public:
        TimerId(Timer* timer) : value_(timer) {}

    private:
        Timer* value_;
    }; // class TimerId

} // namespace muduo


#endif // MUDUO_REACTOR_TIMERID_H