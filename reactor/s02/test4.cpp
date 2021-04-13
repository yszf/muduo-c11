#include "EventLoop.h"
#include "muduo-c11/base/CurrentThread.h"

#include <stdio.h>
#include <unistd.h>


int cnt = 0;
muduo::EventLoop* g_loop = nullptr;

void printTid() {
    printf("pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());
    printf("now %s\n", muduo::Timestamp::now().toString().c_str());
}

void print(const char* msg) {
    printf("msg %s %s, cnt = %d\n", muduo::Timestamp::now().toString().c_str(), msg, cnt);

    if(20 == ++cnt) {
        g_loop->quit();
    }
}

int main() {
    printTid();
    muduo::EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "once1"));
    loop.runAfter(1.5, std::bind(print, "once1.5"));
    loop.runAfter(2.5, std::bind(print, "once2.5"));
    loop.runAfter(3.5, std::bind(print, "once3.5"));

    loop.runEvery(2, std::bind(print, "every2"));
    loop.runEvery(3, std::bind(print, "every3"));

    loop.loop();
    print("main loop exits");
    sleep(1);

    return 0;
}

