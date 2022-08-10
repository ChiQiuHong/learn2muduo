#pragma once

#include "muduo/base/Mutex.h"

#include <pthread.h>

// linux中的条件变量Condition的封装
// 有以下几个函数
// pthread_cond_init() pthread_cond_signal() pthread_broadcast() pthread_cond_wait()
// pthread_cond_destroy()

namespace muduo
{
    class Condition : noncopyable
    {
    public:
        explicit Condition(MutexLock &mutex)
            : mutex_(mutex)
        {
            pthread_cond_init(&cond_, NULL);
        }

        ~Condition()
        {
            pthread_cond_destroy(&cond_);
        }

        void wait()
        {
            pthread_mutex_t* lock = mutex_.getPthreadMutex();
            pthread_cond_wait(&cond_, lock);
        }

        void notify()
        {
            pthread_cond_signal(&cond_);
        }

        void notifyAll()
        {
            pthread_cond_broadcast(&cond_);
        }

    private:
        pthread_cond_t cond_;
        MutexLock &mutex_;
    };
} // namespace muduo