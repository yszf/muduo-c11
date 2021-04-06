#ifndef MUDUO_BASE_SIGLETON_H
#define MUDUO_BASE_SIGLETON_H

#include "muduo-c11/base/noncopyable.h"

#include <pthread.h>

namespace muduo {

    namespace detail {
        template<typename T>
        struct has_no_destroy {
            template<typename C> 
            static char test(decltype(&C::no_destroy));
            template<typename C>
            static int32_t test(...);
            const static bool value = sizeof(test<T>(0)) == 1;
        };
    }

    template<typename T>
    class Singleton : noncopyable {
    public:
        Singleton() = delete;
        ~Singleton() = delete;

        static T& instance() {
            // pthread_onceʵ���˶����Ψһ
            pthread_once(&ponce_, &Singleton::init);
            return *value_;
        }

    private:
        static void init() {
            value_ = new T();
            // ����ʱ���T���Ƿ���no_destroy����
            if (!detail::has_no_destroy<T>::value) {
                ::atexit(destroy);
            }
        }

        static void destroy() {
            // ����ʱ���T�Ƿ���������
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type dummy; (void) dummy;

            delete value_;
        }

        static pthread_once_t   ponce_;
        static T*               value_;

    };

    template<typename T>
    pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

    template<typename T>
    T* Singleton<T>::value_ = nullptr;

} // namespace muduo

#endif // MUDUO_BASE_SINGLETON_H