#include "muduo/net/Timer.h"

using namespace muduo;
using namespace muduo::net;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(system_clock::time_point now)
{
    if (repeat_)
    {
        // 如果是重复执行的，那么到期的时间为当前时间加上定时时间
        duration<double> time(interval_);
        expiration_ = now + duration_cast<nanoseconds>(time);
    }
    else
    {
        // 如果不是，到期时间为非法值，等于停止使用这个定时器。
        system_clock::time_point invalid;
        expiration_ = invalid;
    }
}