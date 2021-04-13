#include "TimerQueue.h"
#include "muduo-c11/base/Timestamp.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <sys/timerfd.h>
#include <iostream>

namespace muduo {

    namespace detail {

        int createTimerfd() {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

            if (timerfd < 0) {
                std::cout << "[createTimerfd] sysfatal: Failed in timerfd_create" << std::endl;
                assert(false);
            }

            return timerfd;
        }

        void resetTimerfd(int timerfd, Timerstamp expiration) {

        }
    }

}

