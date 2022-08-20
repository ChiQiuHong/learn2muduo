#pragma once

#include "muduo/base/CurrentThread.h"
#include "muduo/base/noncopyable.h"
#include <assert.h>
#include <pthread.h>

// 这里是对 pthread_mutex_t 进行简易的封装
// 主要有以下几个函数
// pthread_mutex_init() pthread_mutex_lock() pthread_mutex_unlock() phtread_mutex_destroy()

// CHECK_PTHREAD_RETURN_VALUE
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);  \
                       assert(errnum == 0); (void) errnum; })

namespace muduo
{
    // 互斥锁类
    class MutexLock : noncopyable
    {
    public:
        MutexLock()
            : holder_(0)
        {
            MCHECK(pthread_mutexattr_init(&mutex_attr_));
            MCHECK(pthread_mutexattr_settype(&mutex_attr_, PTHREAD_MUTEX_NORMAL));
            MCHECK(pthread_mutex_init(&mutex_, &mutex_attr_));
        }

        ~MutexLock()
        {
            // 判断是否已经解锁
            assert(holder_ == 0);
            MCHECK(pthread_mutex_destroy(&mutex_));
            MCHECK(pthread_mutexattr_destroy(&mutex_attr_));
        }

        bool isLockedByThisThread() const
        {
            return holder_ == CurrentThread::tid();
        }

        void assertLocked() const
        {
            assert(isLockedByThisThread());
        }

        // 上锁
        void lock()
        {
            MCHECK(pthread_mutex_lock(&mutex_));
            assignHolder();
        }

        // 释放锁
        void unlock()
        {
            unassignHolder();
            MCHECK(pthread_mutex_unlock(&mutex_));
        }

        pthread_mutex_t *getPthreadMutex() // for Condition
        {
            return &mutex_;
        }

    private:
        friend class Condition;

        class UnassignGuard : noncopyable
        {
        public:
            explicit UnassignGuard(MutexLock &owner)
                : owner_(owner)
            {
                owner_.unassignHolder();
            }

            ~UnassignGuard()
            {
                owner_.assignHolder();
            }

        private:
            MutexLock &owner_;
        };

        void unassignHolder()
        {
            holder_ = 0;
        }

        void assignHolder()
        {
            holder_ = CurrentThread::tid();
        }

        pthread_mutex_t mutex_;
        pthread_mutexattr_t mutex_attr_;
        pid_t holder_;
    };

    class MutexLockGuard : noncopyable
    {
    public:
        explicit MutexLockGuard(MutexLock &mutex)
            : mutex_(mutex)
        {
            mutex_.lock();
        }

        ~MutexLockGuard()
        {
            mutex_.unlock();
        }

    private:
        MutexLock &mutex_;
    };

// 定义一个宏，防止产生临时对象，没有锁住临界区
#define MutexLockGuard(x) static_assert(false, "Missing guard object name")

} // namespace muduo