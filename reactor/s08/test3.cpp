#include "EventLoop.h"
#include "Channel.h"

#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

muduo::EventLoop* g_loop;

void timeout(muduo::Timestamp receiveTime) {
    printf("timeout() %s timeout!\n", receiveTime.toFormattedString().c_str());
    g_loop->quit();
}

int main() {
    printf("main() %s started\n", muduo::Timestamp::now().toFormattedString().c_str());

    muduo::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    muduo::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();


    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    loop.loop();

    ::close(timerfd);

    return 0;
}