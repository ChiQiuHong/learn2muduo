#pragma once

#include <inttypes.h>

// 使用gcc __builtin_expect 将最有可能执行的分支告诉编译器
// 节省跳转指令的时间
#define LIKELY(c) (__builtin_expect(!!(c), 1))
#define UNLIKELY(c) (__builtin_expect(!!(c), 0))

namespace muduo
{
    namespace CurrentThread
    {
        // internal
        extern __thread int t_cachedTid;
        extern __thread char t_tidString[32];
        extern __thread int t_tidStringLength;
        extern __thread const char *t_threadName;
        void cacheTid();

        inline int tid()
        {
            if (UNLIKELY(t_cachedTid == 0))
            {
                cacheTid();
            }
            return t_cachedTid;
        }

        inline const char *tidString() // for logging
        {
            return t_tidString;
        }

        inline int tidStringLength() // for logging
        {
            return t_tidStringLength;
        }

        inline const char *name()
        {
            return t_threadName;
        }

        bool isMainThread();

        void sleepUsec(int64_t usec);   // for testing

    } // namespace CurrentThread

} // namespace muduo
