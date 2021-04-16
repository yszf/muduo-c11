#ifndef MUDUO_REACTOR_INETADDRESS_H
#define MUDUO_REACTOR_INETADDRESS_H

#include "muduo-c11/base/copyable.h"

#include <netinet/in.h>
#include <string>

namespace muduo {

    class InetAddress : public copyable {
    public:
        explicit InetAddress(uint16_t port);

        InetAddress(const std::string& ip, uint16_t port);

        InetAddress(const struct sockaddr_in& addr)
            : addr_(addr) {
        }

        std::string toHostPort() const;

        const struct sockaddr_in& getSockAddrInet() const {
            return addr_;
        }

        void setSockAddrInet(const struct sockaddr_in& addr) {
            addr_ = addr;
        }

    private:
        struct sockaddr_in addr_;

    }; // class InetAddress

} // namespace muduo

#endif // MUDUO_REACTOR_INETADDRESS_H