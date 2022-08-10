#pragma once

#include "muduo/base/noncopyable.h"
#include <assert.h>
#include <pthread.h>

// 这里是对 pthread_mutex_t 进行简易的封装
// 主要有以下几个函数
// pthread_mutex_init() pthread_mutex_lock() pthread_mutex_unlock() phtread_mutex_destroy()

namespace muduo
{
    // 互斥锁类
    class MutexLock : noncopyable
    {
    public:
        MutexLock()
            : isLocked_(false)
        {
            pthread_mutex_init(&mutex_, NULL);
        }

        ~MutexLock()
        {
            // 判断是否已经解锁
            assert(!isLocking());
            pthread_mutex_destroy(&mutex_);
        }

        // 上锁
        void lock()
        {
            isLocked_ = true;
            pthread_mutex_lock(&mutex_);
        }

        // 释放锁
        void unlock()
        {
            isLocked_ = false;
            pthread_mutex_unlock(&mutex_);
        }

        bool isLocking() const { return isLocked_; } // 判断锁的状态

        pthread_mutex_t* getPthreadMutex()
        {
            return &mutex_;
        }

    private:
        pthread_mutex_t mutex_;
        bool isLocked_; // 标记是否上锁
    };

    class MutexLockGuard : noncopyable
    {
    public:
        explicit MutexLockGuard(MutexLock& mutex)
            : mutex_(mutex)
        {
            mutex_.lock();
        }

        ~MutexLockGuard()
        {
            mutex_.unlock();
        }

    private:
        MutexLock& mutex_;
    };

// 定义一个宏，防止产生临时对象，没有锁住临界区
#define MutexLockGuard(x) static_assert(false, "Missing guard object name")

} // namespace muduo