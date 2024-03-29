#pragma once

#include "muduo/base/copyable.h"

///
/// 记录定时器Id的抽象类
///

namespace muduo
{
    namespace net
    {

        class Timer;

        ///
        /// An opaque identifier, for canceling Timer
        ///
        class TimerId : public muduo::copyable
        {
        public:
            TimerId()
                : timer_(NULL),
                  sequence_(0)
            {
            }

            TimerId(Timer *timer, int64_t seq)
                : timer_(timer),
                  sequence_(seq)
            {
            }

            // default copy-ctor, dtor and assignment are okay

            friend class TimerQueue;

        private:
            Timer *timer_;      // 指向定时器的指针
            int64_t sequence_;  // 定时器的序号
        };

    } // namespace net

} // namespace muduo
