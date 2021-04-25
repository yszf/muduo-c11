#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include "muduo-c11/base/copyable.h"
#include "muduo-c11/base/Types.h"

namespace muduo {

    namespace net {

        class Timer;

        class TimerId : public copyable {
        public:
            TimerId()
                : timer_(nullptr), 
                sequence_(0) {

            }

            TimerId(Timer* timer, int64_t seq)
                : timer_(timer), 
                sequence_(seq) {

            }

            friend class TimerQueue;

        private:
            Timer* timer_;
            int64_t sequence_;

        }; // class TimerId

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_TIMERID_H