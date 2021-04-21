#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>
#include <unistd.h>

void onConnection(const muduo::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n", conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    }
    else {
        printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
    }
}

void onMessage(const muduo::TcpConnectionPtr& conn, muduo::Buffer* buf, muduo::Timestamp receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n", buf->readableBytes(), conn->name().c_str(), receiveTime.toFormattedString().c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main(int argc, char* argv[]) {
    printf("main(): pid = %d, tid = %d\n", getpid(), muduo::CurrentThread::tid());

    muduo::InetAddress listenAddr(9981);
    muduo::EventLoop loop;
    muduo::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    if (argc > 1) {
        server.setThreadNum(atoi(argv[1]));
    }
    server.start();

    loop.loop();

    return 0;
}