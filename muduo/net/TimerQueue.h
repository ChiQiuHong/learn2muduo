#pragma once

#include <set>
#include <vector>
#include <chrono>

#include "muduo/base/Mutex.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"

using namespace std::chrono;

///
/// TimerQueue用timerfd实现定时，这有别于传统的设置poll/epoll_wait的等待时长的办法
/// TimerQueue用std::map来管理Timer，常用操作的复杂度是O(log N) N是定时器数目
///

namespace muduo
{
    namespace net
    {

        class EventLoop;
        class Timer;
        class TimerId;

        ///
        /// A best efforts timer queue.
        /// No guarantee that the callback will be on time.
        ///
        class TimerQueue : noncopyable
        {
        public:
            explicit TimerQueue(EventLoop *loop);
            ~TimerQueue();

            ///
            /// Schedules the callback to be run at given time,
            /// repeats if @c interval > 0.0.
            ///
            /// Must be thread safe. Usually be called from other threads.
            // addTimer() 供EventLoop使用，EventLoop会把它封装为run*()等函数
            TimerId addTimer(TimerCallback cb,
                             system_clock::time_point when,
                             double interval);

            void cancel(TimerId timerId);

        private:
            // FIXME: use unique_ptr<Timer> instead of raw pointers.
            // This requires heterogeneous comparison lookup (N3465) from C++14
            // so that we can find a T* in a set<unique_ptr<T>>.
            typedef std::pair<system_clock::time_point, Timer *> Entry;
            typedef std::set<Entry> TimerList;
            typedef std::pair<Timer *, int64_t> ActiveTimer;
            typedef std::set<ActiveTimer> ActiveTimerSet;

            void addTimerInLoop(Timer *timer);
            void cancelInLoop(TimerId timerId);
            // called when timerfd alarms
            void handleRead();
            // mvoe out all expired timers
            std::vector<Entry> getExpired(system_clock::time_point now);
            void reset(const std::vector<Entry> &expired, system_clock::time_point now);

            bool insert(Timer *timer);

            EventLoop *loop_;        // TimeQueue所属的EventLoop
            const int timerfd_;      // 定时器文件描述符
            Channel timerfdChannel_; // 文件描述符所对应的事件分发器
            // Timer list sorted by expiration
            TimerList timers_; // 按到期时间先后排序好的定时器队列

            // for cancel()
            ActiveTimerSet activeTimers_;    // 还没到期的定时器队列
            bool callingExpiredTimers_;      // atomic 是否正在处理到期的定时器
            ActiveTimerSet cancelingTimers_; // 待取消的定时器队列
        };

    } // namespace net

} // namespace muduo
