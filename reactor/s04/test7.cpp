#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <unistd.h>

void newConnection(int sockfd, const muduo::InetAddress& peerAddr) {
    printf("newConnection(): accepted a new connection from %s\n", peerAddr.toHostPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    muduo::sockets::close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());

    muduo::EventLoop loop;
    muduo::InetAddress listenAddr(9981);
    muduo::Acceptor acceptor(&loop, listenAddr);

    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();

    return 0;
}

