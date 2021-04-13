#include "EventLoop.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Thread.h"
#include <stdio.h>
#include <unistd.h>

muduo::EventLoop* g_loop = nullptr;

void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());
    g_loop->loop();
}

int main() {
    printf("main(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;
//    muduo::EventLoop loop2;
    g_loop = &loop;

    muduo::Thread thread(threadFunc);
    thread.start();
    thread.join();

    return 0;
}