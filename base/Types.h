#ifndef MUDUO_BASE_TYPES_H
#define MUDUO_BASE_TYPES_H

#include <string.h>
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace muduo {
    using std::string;

    template<typename To, typename From>
    inline To implicit_cast(From const& f) {
        return f;
    }

    template<typename To, typename From>
    inline To down_cast(From* f) {
        if (false) {
            implicit_cast<From*, To>(0);
        }


        #if !defined(NDBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
            assert(f == NULL || dynamic_cast<To>(f) != nullptr);
        #endif
        return static_cast<To>(f);
    }
}

#endif // MUDUO_BASE_TYPES_H