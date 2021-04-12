#include "EventLoop.h"
#include "muduo-c11/base/Thread.h"

muduo::EventLoop* g_loop;

void threadFunc() {
    g_loop->loop();
}

int main() {
    muduo::EventLoop loop;
    g_loop = &loop;
    muduo::Thread t(threadFunc);
    t.start();
    t.join();

    return 0;
}