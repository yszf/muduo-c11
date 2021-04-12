#ifndef MUDUO_REACTOR_TIMERID_H
#define MUDUO_REACTOR_TIMERID_H

#include "muduo-c11/base/copyable.h"

namespace muduo {

    class Timer;
    class TimerId : public copyable {
    public:
        explicit TimerId(Timer* timer)
            : timer_(timer) {}
        
        // default copy-ctor, dtor and assignment are okay
    private:
        Timer* timer_;
    };


} // namespace muduo

#endif // MUDUO_REACTOR_TIMERID_H