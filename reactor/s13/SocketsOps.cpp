#include "SocketsOps.h"

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>

using namespace muduo;

namespace {

    typedef struct sockaddr SA;

    const SA* sockaddr_cast(const struct sockaddr_in* addr) {
        return static_cast<const SA*>(implicit_cast<const void*>(addr));
    }

    SA* sockaddr_cast(struct sockaddr_in* addr) {
        return static_cast<SA*>(implicit_cast<void*>(addr));
    }

    void setNonBlockAndCloseOnExec(int sockfd) {
        // non-block
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);
        // FIXME: check

        // close-on-exec
        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= O_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
        (void) ret;
        // FIXME check
    }
}

int sockets::createNonblockingOrDie() {
#if VALGRIND
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        std::cout << "[sockets::createNonblockingOrDie] sysfatal: sockets:createNonblockingOrDie" << std::endl;
        assert(false);
    }
    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        std::cout << "[sockets::createNonblockingOrDie] sysfatal: sockets:createNonblockingOrDie" << std::endl;
        assert(false);
    }
#endif
    return sockfd;
}

int sockets::connect(int sockfd, const struct sockaddr_in& addr) {
    return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);

    if (ret < 0) {
        std::cout << "[sockets::bindOrDie] trace: sockets:bindOrDie" << std::endl;
        assert(false);
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        std::cout << "[sockets::listenOrDie] trace: sockets:listenOrDie" << std::endl;
        assert(false);
    }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr) {
    socklen_t addrlen = sizeof (*addr);
#if VALGRIND
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0) {
        int savedErrno = errno;
        std::cout << "[sockets::accept] syserr: Socket::accept" << std::endl;

        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
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
                std::cout << "[sockets::accept] fatal: unexpected error of ::accept " << savedErrno << std::endl;
                break;
            default:
                std::cout << "[sockets::accept] fatal: unknown error of ::accept " << savedErrno << std::endl;
                break;
        }
    }

    return connfd;
}

void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        std::cout << "syserr: sockets::close" << std::endl;
        assert(false);
    }
}

void sockets::shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        std::cout << "syserr: sockets::shutdownWrite" << std::endl;
        assert(false);
    }
}

void sockets::toHostPort(char* buf, size_t size, const struct sockaddr_in& addr) {
    char host[INET_ADDRSTRLEN] = "INVALID";
    if (nullptr == ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host))) {
        std::cout << "error: sockets::toHostPort" << std::endl;
        assert(false);
    }

    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);

}

void sockets::fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        std::cout << "error: sockets::fromHostPort" << std::endl;
    //  assert(false);
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        std::cout << "syserr: sockets::getLocalAddr" << std::endl;
        assert(false);
    }
    return localaddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd) {
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = sizeof peeraddr;
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0 ) {
        std::cout << "syserr: sockets::getPeerAddr" << std::endl;
        assert(false);
    }
    return peeraddr;
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = sizeof optval;

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    }
    else {
        return optval;
    }
}

bool sockets::isSelfConnect(int sockfd) {
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    char local[32], peer[32];
    toHostPort(local, sizeof(local), localaddr);
    toHostPort(peer, sizeof(peer), peeraddr);
    std::cout << "local addr = " << local << ", peeraddr = " << peer << std::endl;
    return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}