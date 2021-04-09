#include "muduo-c11/base/Exception.h"
#include "muduo-c11/base/CurrentThread.h"

#include <iostream>

namespace muduo {

    Exception::Exception(string msg) 
        : message_(msg), 
        stack_(CurrentThread::stackTrace(false)) {
        std::cout << "Exception construct" << std::endl;
    } 

} // namespace muduo
