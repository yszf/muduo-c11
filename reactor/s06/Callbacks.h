#ifndef MUDUO_REACTOR_CALLBACKS_H
#define MUDUO_REACTOR_CALLBACKS_H

#include <functional>
#include <memory>

namespace muduo {

    class TcpConnection;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    typedef std::function<void()> TimerCallback;

    typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;

    typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;

    typedef std::function<void (const TcpConnectionPtr&, const char* data, ssize_t len)> MessageCallback;

} // namespace muduo

#endif // MUDUO_REACTOR_CALLBACK_H