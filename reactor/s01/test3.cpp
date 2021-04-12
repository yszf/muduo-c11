#include "EventLoop.h"
#include "Channel.h"
#include <sys/timerfd.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

muduo::EventLoop* g_loop = nullptr;

void timeout() {
    printf("Timeout!\n");
    g_loop->quit();
}

int main() {
    muduo::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    muduo::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enalbleReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 15;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    loop.loop();

    ::close(timerfd);

    return 0;
}

