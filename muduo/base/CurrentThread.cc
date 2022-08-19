#include "muduo/base/CurrentThread.h"

#include <type_traits>
#include <sys/types.h>

namespace muduo
{
    namespace CurrentThread
    {
        __thread int t_cachedTid = 0;
        __thread char t_tidString[32];
        __thread int t_tidStringLength = 6;
        __thread const char *t_threadName = "unknown";
        static_assert(std::is_same<int, pid_t>::value, "pid_t shoule be int");
    } // namespace CurrentThread
    
} // namespace muduo
