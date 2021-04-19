#ifndef MUDUO_REACTOR_TIMERID_H
#define MUDUO_REACTOR_TIMERID_H

#include "muduo-c11/base/copyable.h"
#include "Timer.h"


namespace muduo {

    class TimerId : public copyable {
    public:
        TimerId(Timer* timer) : 
            value_(timer) {

        }

        ~TimerId() {

        }

    private:
        Timer* value_;
        
    }; // class TimerId

} // namespace muduo

#endif // MUDUO_REACTOR_TIMERID_H