#include "muduo-c11/base/Thread.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Types.h"
#include <stdio.h>
#include <unistd.h>
#include <functional>

void mysleep(int seconds) {
    timespec t = { seconds, 0};
    nanosleep(&t, nullptr);
}

void threadFunc() {
    printf("tid = %d\n", muduo::CurrentThread::tid());
}

void threadFunc2(int x) {
    printf("tid = %d, x = %d\n", muduo::CurrentThread::tid(), x);
}

void threadFunc3() {
    printf("tid = %d\n", muduo::CurrentThread::tid());
    mysleep(1);
}

class Foo {
public:
explicit Foo(double x) : x_(x) {}

void memberFunc() {
    printf("tid = %d, Foo::x_ = %f\n", muduo::CurrentThread::tid(), x_);
}

void memberFunc2(const muduo::string& text) {
    printf("tid = %d, Foo::x_ = %f, text = %s\n", muduo::CurrentThread::tid(), x_, text.c_str());
}

private:
    double x_;
}; // class Foo

int main() {
    printf("main(): pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

    muduo::Thread t1(threadFunc);
    t1.start();
    printf("t1.tid = %d\n", t1.tid());
    t1.join();

    muduo::Thread t2(std::bind(threadFunc2, 42), "thread for free function with argument");
    t2.start();
    printf("t2.tid = %d\n", t2.tid());
    t2.join();

    Foo foo(87.53);
    muduo::Thread t3(std::bind(&Foo::memberFunc, std::ref(foo)), "thread for member function without argument");
    t3.start();
    printf("t3.tid = %d\n", t3.tid());
    t3.join();

    muduo::Thread t4(std::bind(&Foo::memberFunc2, std::ref(foo), muduo::string("Shuo Chen")), "thread for member function with one argument");
    t4.start();
    printf("t4.tid = %d\n", t4.tid());
    t4.join();

    {
        muduo::Thread t5(threadFunc3);
        t5.start();
    }

    mysleep(2);

    {
        muduo::Thread t6(threadFunc3);
        t6.start();
        mysleep(2);
    }

    sleep(2);
    printf("number of created threads %d\n", muduo::Thread::numCreated());
    
    return 0;
}