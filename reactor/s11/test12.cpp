#include "Connector.h"
#include "EventLoop.h"

#include <stdio.h>

muduo::EventLoop* g_loop;

void connectCallback(int sockfd) {
    printf("connected.\n");
    g_loop->quit();
}

int main(int argc, char* argv[]) {
    muduo::EventLoop loop;
    g_loop = &loop;
    muduo::InetAddress addr("172.29.104.58", 9981);
    muduo::ConnectorPtr connector(new muduo::Connector(&loop, addr));
    connector->setNewConnectionCallback(connectCallback);
    connector->start();

    loop.loop();

    return 0;
} 
