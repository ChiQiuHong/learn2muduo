#include "muduo/net/EventLoop.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EPoller.h"
#include "muduo/net/Channel.h"
#include "muduo/net/TimerQueue.h"

using namespace muduo;
using namespace muduo::net;

namespace
{   
    // 线程局部变量 指向当前线程内的EventLoop对象
    __thread EventLoop *t_loopInThisThread = 0;
    // 传递给epoll_wait timeout的参数 等待10秒
    const int kEPollTimeMs = 10000;
}

// WHY? __thread 可以直接调用，为什么还需要用这个函数包装
EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
      threadId_(CurrentThread::tid()),
      epoller_(new EPoller()),
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
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (1)
    {
        activeChannels_.clear();
        // 调用 EPoller::epoll()获取当前的活动事件，结果保存到activeChannels_数组里 超时时间 10s
        epoller_->epoll(kEPollTimeMs, &activeChannels_);

        for (Channel* channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent();
        }
        currentActiveChannel_ = NULL;
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->updataChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid(); 
}