#include "muduo-c11/reactor/s00/EventLoop.h"
#include "muduo-c11/base/Thread.h"
#include <stdio.h>
#include <unistd.h>

void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;
    loop.loop();
}

int main() {
    printf("main(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;

    muduo::Thread thread(threadFunc);
    thread.start();

    loop.loop();

    pthread_exit(nullptr);

    return 0;
}