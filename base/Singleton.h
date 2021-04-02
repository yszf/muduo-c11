#ifndef MUDUO_BASE_SIGLETON_H
#define MUDUO_BASE_SIGLETON_H

#include "muduo-c11/base/noncopyable.h"

#include <pthread.h>

namespace muduo {

    template<typename T>
    class Singleton : noncopyable {
    public:
        Singleton() = delete;
        ~Singleton() = delete;

        static T& instance() {

        }

    private:
        static pthread_once_t   ponce_;
        static T*               value_;
    };

    template<typename T>
    pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

    template<typename T>
    T* Singleton<T>::value_ = nullptr;
}

#endif // MUDUO_BASE_SINGLETON_H