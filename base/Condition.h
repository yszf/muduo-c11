#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Mutex.h"

namespace muduo {

    class Condition : noncopyable {
    public:
        explicit Condition(MutexLock& mutex) : mutex_(mutex) {
            int ret = pthread_cond_init(&pcond_, nullptr);
            assert(0 == ret);
            (void) ret;
        }

        ~Condition() {
            int ret = pthread_cond_destroy(&pcond_);
            assert(0 == ret);
            (void) ret;
        }

        void wait() {
            MutexLock::UnassignGuard ug(mutex_);
            int ret = pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
            assert(0 == ret);
            (void) ret;
        }

        // returns true if time out, false otherwise
        bool waitForSeconds(double seconds);

        void notify() {
            int ret = pthread_cond_signal(&pcond_);
            assert(0 == ret);
            (void) ret;
        }

        void notifyAll() {
            int ret = pthread_cond_broadcast(&pcond_);
            assert(0 == ret);
            (void) ret;
        }

    private:
        MutexLock& mutex_;
        pthread_cond_t pcond_;
    }; // class Condition

} // namespace muduo

#endif // MUDUO_BASE_CONDITION_H