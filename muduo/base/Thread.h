#pragma once

#include "muduo/base/Atomic.h"
#include "muduo/base/CountDownLatch.h"

#include <functional>
#include <memory>
#include <pthread.h>
#include <string>

namespace muduo
{

    class Thread : noncopyable
    {
    public:
        typedef std::function<void ()> ThreadFunc;

        explicit Thread(ThreadFunc, const std::string& name = std::string());
        // FIXME: make it movable in C++11
        ~Thread();

        void start();
        int join(); // return pthread_join()

        bool started() const { return started_; }
        // pthread_t pthreadId() const { return pthreadId_; }   // 弃用 见书 4.3节
        pid_t tid() const { return tid_; }
        const std::string& name() const { return name_; }

        static int numCreated() { return numCreated_.get(); }
    
    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
        CountDownLatch latch_;

        static AtomicInt32 numCreated_;
    };

}   // namespace muduo