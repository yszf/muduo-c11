#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace muduo;

EventLoopThread::EventLoopThread()
    : loop_(nullptr),
    thread_(std::bind(&EventLoopThread::threadFunc, this)),
    exiting_(false),
    mutex_(),
    cond_(mutex_) {

}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_!= nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();

    {
        MutexLockGuard lock(mutex_);
        while (nullptr == loop_) {
            cond_.wait();
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    // assert(exiting_);
    MutexLockGuard lock(mutex_);
    loop_ = nullptr;
}