#ifndef MUDUO_REACTOR_CALLBACKS_H
#define MUDDUO_REACTOR_CALLBACKS_H

#include <functional>

namespace muduo {

    typedef std::function<void()> TimerCallback;

    typedef std::function<void()> EventCallback;

} // namespace muduo

#endif // MUDUO_REACTOR_CALLBACKS_H