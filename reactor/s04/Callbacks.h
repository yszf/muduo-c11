#ifndef MUDUO_REACTOR_CALLBACKS_H
#define MUDUO_REACTOR_CALLBACKS_H

#include <functional>

namespace muduo {

    typedef std::function<void()> TimerCallback;

} // namespace muduo

#endif // MUDUO_REACTOR_CALLBACK_H