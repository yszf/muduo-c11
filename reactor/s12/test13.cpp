#include "EventLoop.h"
#include "TcpClient.h"
#include "InetAddress.h"

#include <stdio.h>


int main() {
    muduo::EventLoop loop;
    muduo::InetAddress serverAddress("localhost", 9981);
    muduo::TcpClient client(&loop, serverAddress);


    return 0;
}


