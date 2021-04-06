#include "muduo-c11/base/CurrentThread.h"

namespace muduo {

    namespace CurrentThread {

        __thread int        t_cachedTid = 0;
        __thread char       t_tidString[32];
        __thread int        t_tidStringLength;
        __thread const char* t_threadName = "unknown";

        static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

    } // namespace CurrentThread

} // namespace muduo