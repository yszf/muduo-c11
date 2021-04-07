#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Mutex.h"
#include "muduo-c11/base/Condition.h"

namespace muduo {

    class CountDownLatch : noncopyable {
    public:
        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount() const;

    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
    }; // class CountDownLatch

} // namespace muduo

#endif // MUDUO_BASE_COUNTDOWNLATCH_H