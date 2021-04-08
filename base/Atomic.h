#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include "muduo-c11/base/noncopyable.h"
#include <stdint.h>

namespace muduo {

    namespace detail {

        template<typename T>
        class AtomicIntergerT : noncopyable {
        public:
            AtomicIntergerT() : value_(0) {}

            T get() {
                return __sync_val_compare_and_swap(&value_, 0, 0);
            }

            T getAndAdd(T x) {
                return __sync_fetch_and_add(&value_, x);
            }

            T addAndGet(T x) {
                // return getAndAdd(x) + x;
                return __sync_add_and_fetch(&value_, x);
            }

            T incrementAndGet() { // 自增
                return addAndGet(1);
            }

            T decrementAndGet() { // 自减
                return addAndGet(-1);
            }

            T getAndSet(T newValue) {
                return __sync_lock_test_and_set(&value_, newValue);
            } 

        private:
            volatile T value_;
        }; 
    } // namespace detail

    typedef detail::AtomicIntergerT<int32_t> AtomicInt32;
    typedef detail::AtomicIntergerT<int64_t> AtomicInt64;

} // namespace muduo

#endif