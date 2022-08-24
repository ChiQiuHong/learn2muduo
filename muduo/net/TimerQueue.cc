#include "muduo/net/TimerQueue.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Timer.h"
#include "muduo/net/TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace muduo
{
    namespace net
    {
        namespace detail
        {
            ///
            /// 调用 timer_create() 创建特定的定时器
            /// CLOCK_MONOTONIC 以固定的速率运行，系统启动后不会被改变
            /// TFD_NONBLOCK 设置文件描述符非阻塞
            /// TFD_CLOEXEC 设置文件描述符close-on-exec，即使用exec执行程序时，关闭该文件描述符
            ///
            int createTimerfd()
            {
                int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                               TFD_NONBLOCK | TFD_CLOEXEC);
                if (timerfd < 0)
                {
                    LOG_FATAL << "Failed in timer_create"; // FIXME: LOG_SYSFATAL
                }
                return timerfd;
            }

            struct timespec howMuchTimeFromNow(system_clock::time_point when)
            {
                // time_since_epoch() 单位为 纳秒
                auto diff = when.time_since_epoch() - system_clock::now().time_since_epoch();
                int64_t us = duration_cast<microseconds>(diff).count();

                if (us < 100)
                {
                    us = 100;
                }

                struct timespec ts;
                ts.tv_sec = static_cast<time_t>(us / 1000000);
                ts.tv_nsec = static_cast<long>((us % 1000000) * 1000);

                return ts;
            }

            // 当定时器到期后，需要对timerfd进行read操作以告知内核在fd上的事件已接受并处理
            // 否则可能被Poller重新监听
            void readTimerfd(int timerfd, system_clock::time_point now)
            {
                uint64_t howmany;
                ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
                LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at "; // TODO << now 重载操作符输出时间
                if (n != sizeof(howmany))
                {
                    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes insterd of 8";
                }
            }

            // 设置定时器
            // struct itimerspec {
            // struct timespec it_interval;    // 触发到期的间隔时间
            // struct timespec it_value;       // 初始到期时间
            // };
            void resetTimerfd(int timerfd, system_clock::time_point expiration)
            {
                struct itimerspec newValue; // 指定定时器的初始到期时间和间隔
                struct itimerspec oldValue; // 返回定时器这次设置之前的到期时间
                memset(&newValue, 0, sizeof(newValue));
                memset(&oldValue, 0, sizeof(oldValue));
                // it_value 表示启动定时器后，定时器第一次定时到期的时间
                newValue.it_value = howMuchTimeFromNow(expiration);
                int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
                if (ret)
                {
                    LOG_ERROR << "timerfd_settime()"; // FIXME LOG_SYSERR
                }
            }

        } // namespace detail

    } // namespace net

} // namespace muduo

using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::detail;

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(detail::createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(
        std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    // TODO 取消注册timerfd事件 从EventLoop、Poller 移除监听
    // timerfdChannel_.disableAll();
    // timerfdChannel_.remove();
    ::close(timerfd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (const Entry &timer : timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             system_clock::time_point when,
                             double interval)
{
    Timer *timer = new Timer(std::move(cb), when, interval);
    // 添加到定时器
    // 调用了EventLoop::runInLoop() 作为计算任务放入队列中在loop函数中执行 保证线程安全
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    loop_->assertInLoopThread();
    // 判断是否需要修改定时器
    bool earliestChanged = insert(timer);

    if (earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    // 通过 TimerId 从未超时的队列中 activeTimers_ 查找 timer
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end())
    {
        // 从定时器中队列删除 Timer 并且释放 Timer 对象
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void)n;
        delete it->first; // FIXME: no delete please
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    system_clock::time_point now = system_clock::now();
    // 对timefd进行读操作 避免重复触发到期事件
    readTimerfd(timerfd_, now);

    // 获取当前时刻到期的定时器列表
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true; // 开始处理到期的定时器
    cancelingTimers_.clear();     // 清理要取消的定时器

    for (const Entry &it : expired)
    {
        it.second->run(); // 执行回调函数
    }

    callingExpiredTimers_ = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(system_clock::time_point now)
{
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    // lower_bound() 二分查找 这里返回的是第一个未到期的 Timer 的迭代器
    TimerList::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    // 拷贝到期的迭代器
    std::copy(timers_.begin(), end, back_inserter(expired));
    // 从定时器序列中删除到期的定时器
    timers_.erase(timers_.begin(), end);

    // 从 activeTimers_中移除到期的定时器
    for (const Entry &it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1);
        (void)n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, system_clock::time_point now)
{
    // 重置定时器 两种情况
    // 1) 需要重复执行，重新设定超时事件，并作为新的定时器添加到activeTimers_中
    // 2) 不需要重复执行，销毁Timer对象即可
    system_clock::time_point nextExpire;

    for (const Entry &it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        // 需要重复执行并且不是待取消的定时器
        if (it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            // FIXME move to a free list
            delete it.second; // FIXME: no delete please
        }

        // 重置定时器有可能会使定时器队列里最早到期的定时器发生改变
        // 需要更新timerfd
        if(!timers_.empty()) {
            nextExpire = timers_.begin()->second->expiration();
        }
        system_clock::time_point invalid;
        if(nextExpire > invalid) {
            detail::resetTimerfd(timerfd_, nextExpire);
        }
    }
}

bool TimerQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;                        // 最早到期的时间是否发生改变
    system_clock::time_point when = timer->expiration(); // 新加入的定时器的到期时间
    TimerList::iterator it = timers_.begin();
    // 新加入的定时器的到期时间要小于现有最先到期的定时器的到期时间
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    // 添加到timers_中
    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second);
        (void)result;
    }
    // 添加到 activeTimers_ 中
    {
        std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}