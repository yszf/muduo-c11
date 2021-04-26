#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include "muduo-c11/base/Timestamp.h"
#include <functional>
#include <memory>

namespace muduo {

    namespace net {

        typedef std::function<void()> TimerCallback;

        class TcpConnection;
        class Buffer;

        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

        typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;

        typedef std::function<void(const TcpConnectionPtr&, Buffer*, muduo::Timestamp)> MessageCallback;

        typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;

        typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

        typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_CALLBACKS_H