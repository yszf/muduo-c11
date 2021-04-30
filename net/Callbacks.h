#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include "muduo-c11/base/Timestamp.h"
#include <functional>
#include <memory>

namespace muduo {

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_2;

    template<typename T>
    inline T* get_pointer(const std::shared_ptr<T>& ptr) {
        return ptr.get();
    }

    template<typename T>
    inline T* get_pointer(const std::unique_ptr<T>& ptr) {
        return ptr.get();
    }

    namespace net {

        typedef std::function<void()> TimerCallback;

        class TcpConnection;
        class Buffer;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

        typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;

        typedef std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)> MessageCallback;

        typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;

        typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

        typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

        void defaultConnectionCallback(const TcpConnectionPtr& conn);

        void defaultMessageCallback(const 
        TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_CALLBACKS_H