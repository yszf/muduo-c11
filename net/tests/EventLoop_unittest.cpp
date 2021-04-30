#include "muduo-c11/net/EventLoop.h"
#include "muduo-c11/base/Thread.h"

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;

void callback() {
    printf("callback(): pid = %d, tid = %d\n", ::getpid(), CurrentThread::tid());
    EventLoop anotherLoop;
}

void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", ::getpid(), CurrentThread::tid());
    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
    loop.runAfter(1.0, callback);
    loop.loop();
}

int main() {
    printf("main(): pid = %d, tid = %d\n", ::getpid(), CurrentThread::tid());

    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

    Thread thread(threadFunc);
    thread.start();

    loop.loop();

    return 0;
}