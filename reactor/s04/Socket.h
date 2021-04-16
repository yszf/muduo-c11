#ifndef MUDUO_REACTOR_SOCKET_H
#define MUDUO_REACTOR_SOCKET_H

#include "muduo-c11/base/noncopyable.h"

namespace muduo {

    class InetAddress;
    class Socket : noncopyable {
    public:
        explicit Socket(int sockfd) 
            : sockfd_(sockfd) {

        }

        ~Socket();

        int fd() const {
            return sockfd_;
        }

        void bindAddress(const InetAddress& localaddr);

        void listen();

        int accept(InetAddress* peeraddr);

        void setReuseAddr(bool on);

    private:
        const int sockfd_;
        
    }; // class Socket

} // namespace muduo

#endif // MUDUO_REACTOR_SOCKET_H