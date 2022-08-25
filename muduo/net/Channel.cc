#include "muduo/net/Channel.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <sys/epoll.h>
#include <sstream>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd__)
    : loop_(loop),
      fd_(fd__),
      events_(0),
      revents_(0),
      index_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::handleEvent(system_clock::time_point receiveTime)
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (tied_)
        {
            guard = tie_.lock();
            if (guard)
            {
                handleEventWithGuard(receiveTime);
            }
        }
        else
        {
            handleEventWithGuard(receiveTime);
        }
    }
}

void Channel::handleEventWithGuard(system_clock::time_point receiveTime)
{
    eventHandling_ = true;
    LOG_TRACE << reventsToString();

    // 当事件为挂起并没有可读事件时
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (logHup_)
        {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() EPOLLHUP";
        }
        if (closeCallback_)
            closeCallback_();
    }

    // 发生错误
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
            errorCallback_();
    }

    // 发生可读事件
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLHUP))
    {
        if (readCallback_)
            readCallback_(receiveTime);
    }

    // 发生可写事件
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }

    eventHandling_ = false;
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

std::string Channel::reventsToString() const
{
    return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const
{
    return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLHUP)
        oss << "HUP ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLERR)
        oss << "ERR ";

    return oss.str();
}
