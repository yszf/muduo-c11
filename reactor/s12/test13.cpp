#include "EventLoop.h"
#include "TcpClient.h"
#include "InetAddress.h"

#include <stdio.h>

std::string message = "Hello";

void onConnection(const muduo::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n", conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
        conn->send(message);
    }
    else {
        printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
    }
}

void onMessage(const muduo::TcpConnectionPtr& conn, muduo::Buffer* buf, muduo::Timestamp receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n", buf->readableBytes(), conn->name().c_str(), receiveTime.toFormattedString().c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main() {
    muduo::EventLoop loop;
    muduo::InetAddress serverAddress("127.0.0.1", 9981);
    muduo::TcpClient client(&loop, serverAddress);

    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.enableRetry();
    client.connect();
    loop.loop();

    return 0;
}


