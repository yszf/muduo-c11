#include "muduo-c11/base/Singleton.h"
#include "muduo-c11/base/Types.h"
#include "muduo-c11/base/Thread.h"
#include "muduo-c11/base/CurrentThread.h"
#include <stdio.h>

class Test : muduo::noncopyable {
public:
 //   void no_destroy();

    Test() {
        printf("tid = %d, constructing Test %p\n", muduo::CurrentThread::tid(), this);
    }

    ~Test() {
        printf("tid = %d, destructing Test %p %s\n", muduo::CurrentThread::tid(), this, name_.c_str());
    }

    const muduo::string& name() const { return name_; }
    void setName(const muduo::string& n) { name_ = n; }

private:
    muduo::string name_;
}; // class Test

class TestNoDestroy : muduo::noncopyable {
public:
    void no_destroy();

    TestNoDestroy() {
        printf("tid = %d, constructing TestNoDestroy %p\n", muduo::CurrentThread::tid(), this);
    }

    ~TestNoDestroy() {
        printf("tid = %d, destructing TestNoDestroy %p\n", muduo::CurrentThread::tid(), this);
    }

}; // class TestNoDestroy

void threadFunc() {
    printf("tid = %d, %p name = %s\n", muduo::CurrentThread::tid(), &muduo::Singleton<Test>::instance(), muduo::Singleton<Test>::instance().name().c_str());
    muduo::Singleton<Test>::instance().setName("only one, changed");
}

int main() {
    muduo::Singleton<Test>::instance().setName("only one");
    muduo::Thread t1(threadFunc);
    t1.start();
    t1.join();
    printf("tid = %d, %p name = %s\n", muduo::CurrentThread::tid(), &muduo::Singleton<Test>::instance(), muduo::Singleton<Test>::instance().name().c_str());

    muduo::Singleton<TestNoDestroy>::instance();
    printf("with valgrind, you should see %zd-byte memory leak.\n", sizeof(TestNoDestroy));

    return 0;
}