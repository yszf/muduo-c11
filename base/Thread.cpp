#include "muduo-c11/base/Thread.h"
#include "muduo-c11/base/CurrentThread.h"
#include "muduo-c11/base/Exception.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <iostream>

namespace muduo {

    namespace detail {

        pid_t gettid() {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }

        void afterFork() {
            CurrentThread::t_cachedTid = 0;
            CurrentThread::t_threadName = "main";
            CurrentThread::tid();
        }

        class ThreadNameInitializer {
        public:
            ThreadNameInitializer() {
                CurrentThread::t_threadName = "main";
                CurrentThread::tid();
                pthread_atfork(nullptr, nullptr, &afterFork);
            }
        }; // class ThreadNameInitializer

        ThreadNameInitializer init;

        struct ThreadData {
            typedef muduo::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            string name_;
            pid_t* tid_;
            CountDownLatch* latch_;
            
            ThreadData(ThreadFunc func, const string& name, 
             pid_t* tid, CountDownLatch* latch)
             : func_(std::move(func)), 
             name_(name), 
             tid_(tid), 
             latch_(latch) {
                
            }

            void runInThread() {
                *tid_ = CurrentThread::tid();
                tid_ = nullptr;

                latch_->countDown();
                latch_ = nullptr;

                CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();

                ::prctl(PR_SET_NAME, CurrentThread::t_threadName);
                try {
                    func_();
                    CurrentThread::t_threadName = "finished";
                }
                catch (const Exception& ex) {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                    abort();
                }
                catch (const std::exception& ex) {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    abort();
                }
                catch(...) {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                    throw;
                }
            }
        }; // struct ThreadData

        void* startThread(void* obj) {
            ThreadData* data = static_cast<ThreadData*>(obj);
            data->runInThread();
            delete data;
            return nullptr;
        }

    } // namespace detail

    void CurrentThread::cacheTid() {
        if (0 == t_cachedTid) {
            t_cachedTid = detail::gettid();
            t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
        }
    }

    bool CurrentThread::isMainThread() {
        return tid() == ::getpid();
    }

    void CurrentThread::sleepUsec(int64_t usec) {
        struct timespec ts = { 0, 0};
        const int64_t kMicroSecondsPerSecond = 1000000;
        ts.tv_sec = static_cast<time_t>(usec / kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(usec % kMicroSecondsPerSecond * 1000);
        ::nanosleep(&ts, nullptr);
    }

    AtomicInt32 Thread::numCreated_;

    Thread::Thread(ThreadFunc func, const string& n) 
        : started_(false), 
         joined_(false),
         pthreadId_(0),
         tid_(0),
         func_(std::move(func)),
         name_(n),
         latch_(1) {
        setDefaultName();
    }

    Thread::~Thread() {
        if (started_ && !joined_) {
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

        detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);

        if (pthread_create(&pthreadId_, nullptr, &detail::startThread, data)) {
            std::cout << "failed in pthread_create!" << std::endl;
            started_ = false;
            delete data;
        }
        else {
            latch_.wait();
            assert(tid_ > 0);
        }
    }

    int Thread::join() {
        assert(started_);
        assert(!joined_);
        joined_= true;
        return pthread_join(pthreadId_, nullptr);
    }

} // namespace muduo