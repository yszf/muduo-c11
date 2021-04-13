#include "EventLoop.h"
#include "Channel.h"
#include "muduo-c11/base/CurrentThread.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

muduo::EventLoop* g_loop = nullptr;

void timeout() {
    printf("timeout!");
    g_loop->quit();
}


int main() {
    printf("main(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

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