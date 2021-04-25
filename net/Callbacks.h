#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include <functional>


namespace muduo {

    namespace net {

        typedef std::function<void()> TimerCallback;

    }

}

#endif // MUDUO_NET_CALLBACKS_H