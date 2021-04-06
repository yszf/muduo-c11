#include "muduo-c11/base/Thread.h"

#include <unistd.h>
#include <syscall.h>
#include <stdio.h>

namespace muduo {

    namespace detail {

        pid_t gettid() {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }


        struct ThreadData {
            typedef muduo::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            string name_;
            pid_t* tid_;
            
            ThreadData(ThreadFunc func, const string& name, pid_t* tid)
             : func_(std::move(func)), 
             name_(name),
             tid_(tid) {

            }

            void runInThread() {
                
            }
        };
    }

    AtomicInt32 Thread::numCreated_;

    Thread::Thread(ThreadFunc func, const string& n) 
        : started_(false), 
         joined_(false),
         pthreadId_(0),
         tid_(0),
         func_(std::move(func)),
         name_(n) {
        setDefaultName();
    }

    Thread::~Thread() {
        if (started && !joined_) {
            pthread_detach(pthreadId_);
        }
    }

    void Thread::setDefaultName() {
        int num = numCreated_.incrementAndGet();
        if (name_.empty()) {
            char buf[32];
            snprintf(buf, sizeof buf, "Thread%d", num);
            name_ = buf;
        }
    }

    void Thread::start() {
        assert(!started_);
        started_ = true;

        if (pthread_create(&pthreadId_, nullptr, &detail::startThread, data)) {

        }
    }

} // namespace muduo