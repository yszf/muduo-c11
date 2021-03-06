#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"


using namespace muduo;
using namespace muduo::net;


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& name) 
    : baseLoop_(baseLoop), 
    name_(name), 
    started_(false), 
    numThreads_(0), 
    next_(0) {

}

EventLoopThreadPool::~EventLoopThreadPool() {
    // Don't delete loop, it's statck variable
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;
    for (int i =0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
        loops_.emplace_back(t->startLoop());
    }

    if (0 == numThreads_ && cb) {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop =  baseLoop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (implicit_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else {
        return loops_;
    }
}