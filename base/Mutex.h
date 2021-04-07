#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/CurrentThread.h"
#include <pthread.h>
#include <assert.h>

namespace muduo {

    class MutexLock : noncopyable {
    public:
        MutexLock() : holder_(0) {
            int ret = pthread_mutex_init(&mutex_, nullptr);
            assert(0 == ret);
            (void) ret;
        }

        ~MutexLock() {
            assert(holder_ == 0);
            int ret = pthread_mutex_destroy(&mutex_);
            assert(0 == ret);
            (void) ret;
        }

        bool isLockedByThisThread() const {
            return holder_ == CurrentThread::tid();
        }

        void assertLocked() const {
            assert(isLockedByThisThread());
        }

        void lock() {
            pthread_mutex_lock(&mutex_);
            assignHolder();
        }

        void unlock() {
            unassignHolder();
            pthread_mutex_unlock(&mutex_);
        }

        pthread_mutex_t* getPthreadMutex() {
            return &mutex_;
        }

    private:
        friend class Condition;

        class UnassignGuard : noncopyable {
        public:
            UnassignGuard(MutexLock& owner) : owner_(owner) {
                owner_.unassignHolder();
            }

            ~UnassignGuard() {
                owner_.assignHolder();
            }
        private:
            MutexLock& owner_;
        }; // class UnassignGuard

        void unassignHolder() {
            holder_ = 0;
        }

        void assignHolder() {
            holder_ = CurrentThread::tid();
        }

        pthread_mutex_t     mutex_;
        pid_t               holder_;

    }; // class MutexLock 

    class MutexLockGuard : noncopyable {
    public:
        MutexLockGuard(MutexLock& mutex) : mutex_(mutex){
            mutex_.lock();
        }

        ~MutexLockGuard() {
            mutex_.unlock();
        }
    private:
        MutexLock& mutex_;
    }; // class MutexLockGuard


} // namespace muduo

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

#endif // MUDUO_BASE_MUTEX_H