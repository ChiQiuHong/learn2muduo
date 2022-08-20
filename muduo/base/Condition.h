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
            MCHECK(pthread_cond_init(&pcond_, NULL));
        }

        ~Condition()
        {
            MCHECK(pthread_cond_destroy(&pcond_));
        }

        void wait()
        {
            MutexLock::UnassignGuard ug(mutex_);
            pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
        }

        // returns true if time out, false otherwise.
        bool waitForSeconds(double seconds);

        void notify()
        {
            MCHECK(pthread_cond_signal(&pcond_));
        }

        void notifyAll()
        {
            MCHECK(pthread_cond_broadcast(&pcond_));
        }

    private:
        MutexLock &mutex_;
        pthread_cond_t pcond_;
    };
} // namespace muduo