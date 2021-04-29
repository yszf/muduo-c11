#include "SocketsOps.h"
#include "muduo-c11/base/Types.h"
#include "Endian.h"

#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

namespace {

    typedef struct sockaddr SA;

    
    void setNonBlockAndCloseOnExec(int sockfd) {
        // non-block
        int flags = ::fcntl(sockfd, F_GETFL,0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);
        assert(0 == ret);

        // close-on-exec
        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
        assert(0 == ret);
    }

} // namespace

int sockets::createNonblockingOrDie(sa_family_t family) {
#if VALGRIND
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        std::cout << "[sockets::createNonblockingOrDie] sysfatal" << std::endl;
        assert(false);
    }
    setNonBlockAndCloseOnExec(sockfd);
#else 
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        std::cout << "[sockets::createNonblockingOrDie] sysfatal" << std::endl;
        assert(false);
    }
#endif
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        std::cout << "[sockets::bindOrDie] sysfatal" << std::endl;
        assert(false);
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        std::cout << "[sockets::listenOrDie]" << std::endl;
        assert(false);
    }
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined (NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else 
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0) {
        int savedErrno = errno;
        std::cout << "[sockets::accept] syserr" << std::endl;
        switch (savedErrno) {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            std::cout << "unexpected error of ::accept " << savedErrno << std::endl;
            break;
        default:
            std::cout << "unknown error of ::accept " << savedErrno << std::endl;
            break;
        }
    }
    return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t sockets::read(int sockfd, void* buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec* iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void* buf, size_t count) {
    return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        std::cout << "[sockets::close] syserr" << std::endl;
        assert(false);
    }
}

void sockets::shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        std::cout << "[sockets::shutdownWrite] syserr" << std::endl;
        assert(false);
    }
}

void sockets::toIpPort(char* buf, size_t size, const struct sockaddr* addr) {
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = sockets::networkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size, const struct sockaddr* addr) {
    if (AF_INET == addr->sa_family) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (AF_INET6 == addr->sa_family) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
    else {
        std::cout << "[sockets::toIp] Unknown family " << addr->sa_family << std::endl;
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        std::cout << "[sockets::fromIpPort] sockaddr_in syserr" << std::endl;
        assert(false);
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        std::cout << "[sockets::fromIpPort] sockaddr_in6 syserr" << std::endl;
        assert(false);
    }
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    }
    else {
        return optval;
    }
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr) {
    return  static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr) {
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localAddr;
    memZero(&localAddr, sizeof localAddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localAddr);
    if (::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0) {
        std::cout << "[sockets::getLocalAddr] syserr" << std::endl;
        assert(false);
    }
    return localAddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peerAddr;
    memZero(&peerAddr, sizeof peerAddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peerAddr);
    if (::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrlen) < 0) {
        std::cout << "[sockets::getPeerAddr] syserr" << std::endl;
        assert(false);
    }
    return peerAddr;
}

bool sockets::isSelfConnect(int sockfd) {
    struct sockaddr_in6 localAddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peerAddr = getPeerAddr(sockfd);
    if (AF_INET == localAddr.sin6_family) {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localAddr);

        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peerAddr);
        return laddr4->sin_port == raddr4->sin_port && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    }
    else if (AF_INET6 == localAddr.sin6_family) {
        return localAddr.sin6_port == peerAddr.sin6_port && memcmp(&localAddr.sin6_addr, &peerAddr.sin6_addr, sizeof localAddr.sin6_addr) == 0;
    }
    else {
        return false;
    }
}