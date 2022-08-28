#include "muduo/net/EventLoop.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EPoller.h"
#include "muduo/net/Channel.h"
#include "muduo/net/TimerQueue.h"

#include <algorithm>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{   
    // 线程局部变量 指向当前线程内的EventLoop对象
    __thread EventLoop *t_loopInThisThread = 0;
    // 传递给epoll_wait timeout的参数 等待10秒
    const int kEPollTimeMs = 10000;

    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            LOG_ERROR << "Failed in eventfd"; // FIXME: LOG_SYSERR
            abort();
        }
        return evtfd;
    }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(CurrentThread::tid()),
      epoller_(EPoller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(NULL)
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    // 如果当前线程已经存在EventLoop了，终止程序
    if(t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop "  << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    // 设置唤醒 IO 线程的可读时间回调，并注册到 EPoller中
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false; // FIXME: what if someone calls quit() bufore loop() ?
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_)
    {
        activeChannels_.clear();
        // 调用 EPoller::epoll()获取当前的活动事件，结果保存到activeChannels_数组里 超时时间 10s
        epollReturnTime_ = epoller_->epoll(kEPollTimeMs, &activeChannels_);
        ++iteration_;
        if (Logger::logLevel() <= Logger::TRACE)
        {
            printActiveChannels();
        }
        // TODO sort channel by priority
        eventHandling_ = true;
        for (Channel* channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(epollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
        LOG_TRACE << "Run in Loop";
    }
    else
    {
        // 如果不在 IO 线程里，就先放入到任务队列中
        LOG_TRACE << "Run in queueInLoop";
        queueInLoop(std::move(cb));
    }
}

// 唤醒IO线程 两种情况
// 1) 如果调用queueInLoop()的线程不是IO线程，那么唤醒
// 2) 如果在IO线程调用queueInLoop()，而此时正在调用pendingFunctor
//  换句话说，只有在IO线程的事件回调中调用queueInLoop()才无须wakeup
void EventLoop::queueInLoop(Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        // 由于doPendingFunctors() 调用的 Functor 可能再调用
        // queueInLoop(cb), 这时 queueInLoop() 就必须 wakeup()
        // 否则这些新加入的 cb 就不能被及时调用了。
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

TimerId EventLoop::runAt(system_clock::time_point time, TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}
            
TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    duration<double> delay_time(delay);
    system_clock::time_point time =  system_clock::now() + duration_cast<nanoseconds>(delay_time);
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    duration<double> delay_time(interval);
    system_clock::time_point time =  system_clock::now() + duration_cast<nanoseconds>(delay_time);
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}

void EventLoop::wakeup()
{
    // 通过往eventfd写标志通知，让阻塞poll立马返回并执行回调函数
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << "bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->updataChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
            std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());   
    }
    epoller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return epoller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid(); 
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << "bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    // 减少了临界区的长度 避免死锁（Functor可能会再调用queueInLoop()）
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
    for (const Channel* channel : activeChannels_)
    {
        LOG_TRACE << "{" << channel->reventsToString() << "} ";
    }
}