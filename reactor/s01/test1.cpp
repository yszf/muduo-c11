#include "EventLoop.h"
#include "muduo-c11/base/Thread.h"
#include <stdio.h>
#include <sys/unistd.h>

void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;
    loop.loop();
}


int main() {
    printf("main(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;

    muduo::Thread thread(nullptr);
    thread.start();

    loop.loop();

    pthread_exit(nullptr);

    return 0;
}
