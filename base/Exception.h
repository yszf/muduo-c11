#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H

#include "muduo-c11/base/Types.h"
#include <exception>

namespace muduo {

    class Exception : public std::exception {
        public:
            Exception(string msg);
            ~Exception() noexcept override = default;

            const char* what() const noexcept override {
                return message_.c_str();
            }

            const char* stackTrace() const noexcept {
                return stack_.c_str();
            }

        private:
            string message_;
            string stack_;

    }; // class Exception

} // namespace muduo

#endif // MUDUO_BASE_EXCEPTION_H