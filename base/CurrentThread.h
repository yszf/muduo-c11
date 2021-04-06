#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include "muduo-c11/base/Types.h"

namespace muduo {

    namespace CurrentThread {

        extern __thread int         t_cachedTid;
        extern __thread char        t_tidString[32];
        extern __thread int         t_tidStringLength;
        extern __thread const char* t_threadName;

        void cacheTid();

        bool isMainThread();

        void sleepUsec(int64_t usec);

        string statckTrace(bool demangle);

        inline int tid() {
            //  if (value) 等价于 if (__builtin_expert(value, x)),与x的值无关.
            if (__builtin_expect(t_cachedTid == 0, 0)) {
                cacheTid();
            }
            return t_cachedTid;
        }

        inline const char* tidString() {
            return t_tidString;
        }

        inline int tidStringLength() {
            return t_tidStringLength;
        }

        inline const char* name() {
            return t_threadName;
        }

    } // namespace CurrentThread

} // namespace muduo

#endif // MUDUO_BASE_CURRENTTHREAD_H