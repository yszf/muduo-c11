#ifndef MUDUO_REACTOR_CALLBACKS_H
#define MUDUO_REACTOR_CALLBACKS_H

#include "muduo-c11/base/Timestamp.h"

#include <functional>
#include <memory>

namespace muduo {

    class Buffer;
    class TcpConnection;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    typedef std::function<void()> TimerCallback;

    typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;

    typedef std::function<void (const TcpConnectionPtr&, Buffer* buff, Timestamp)> MessageCallback;

    typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;

    typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;


} // namespace muduo

#endif // MUDUO_REACTOR_CALLBACK_H