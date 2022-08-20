#pragma once

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo
{
    class CountDownLatch : noncopyable
    {
    public:
        explicit CountDownLatch(int count); // 倒数几次

        void wait(); // 等待计数值变为 0

        void countDown(); // 计数减 1

        int getCount() const;

    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
    };
} // namespace muduo
