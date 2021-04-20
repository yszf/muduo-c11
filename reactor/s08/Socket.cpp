#include "Socket.h"
#include "SocketsOps.h"
#include "InetAddress.h"


using namespace muduo;

Socket::~Socket() {
    sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr) {
    sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    assert(0 == ret);
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}