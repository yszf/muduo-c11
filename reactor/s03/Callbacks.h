#ifndef MUDUO_REACTOR_CALLBACK_H
#define MUDUO_REACTOR_CALLBACK_H

#include <functional>

namespace muduo {

    // All client visible callbacks go here
    typedef std::function<void()> TimerCallback;

} // namespace muduo


#endif // MUDUO_REACTOR_CALLBACK_H