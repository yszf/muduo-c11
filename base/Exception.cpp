#include "muduo-c11/base/Exception.h"
#include "muduo-c11/base/CurrentThread.h"

namespace muduo {

    Exception::Exception(string msg) 
        : message_(msg), 
        stack_(CurrentThread::statckTrace(false)) {

    }

} // namespace muduo
