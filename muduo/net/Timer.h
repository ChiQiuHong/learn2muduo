#pragma once

#include "muduo/base/Atomic.h"
#include "muduo/net/Callbacks.h"

#include <chrono>

using namespace std::chrono;

///
/// 定时器 内部类
/// 封装了定时器的一些参数，例如超时回调函数、超时时间
/// 定时器是否重复、重复时间间隔、定时器的序列号
///

namespace muduo
{
    namespace net
    {

        ///
        /// Internal class for timer event.
        ///
        class Timer : noncopyable
        {
        public:
            Timer(TimerCallback cb, system_clock::time_point when, double interval)
                : callback_(std::move(cb)),
                  expiration_(when),
                  interval_(interval),
                  repeat_(interval > 0.0),
                  sequence_(s_numCreated_.incrementAndGet())
            {
            }

            // 时间到了执行回调函数
            void run() const
            {
                callback_();
            }

            system_clock::time_point expiration() const { return expiration_; }
            bool repeat() const { return repeat_; }
            int64_t sequence() const { return sequence_; }

            // 重启定时器
            void restart(system_clock::time_point now);

            // 返回当前定时器数目
            static int64_t numCreated() { return s_numCreated_.get(); }

        private:
            const TimerCallback callback_; // 定时器回调函数
            system_clock::time_point expiration_;                   // 定时器到期的时间
            const double interval_;        //定时器重复执行的时间间隔，如不重复，设为非正值
            const bool repeat_;            // 定时器是否需要重复执行
            const int64_t sequence_;       // 定时器的序号

            static AtomicInt64 s_numCreated_; // 定时器的个数
        };

    } // namespace net

} // namespace muduo
