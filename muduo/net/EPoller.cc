#include "muduo/net/EPoller.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"

#include <sys/epoll.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

EPoller::EPoller()
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL << "EPoller::EPoller";
    }
}

EPoller::~EPoller()
{
    ::close(epollfd_);
}

void EPoller::epoll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_TRACE << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
    }
    else if(numEvents == 0)
    {
        LOG_TRACE << "nothing happened";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_ERROR << "EPoller::epoll()";
        }
    }
}

void EPoller::updataChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_[fd] = channel;
    update(EPOLL_CTL_ADD, channel);
}

void EPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

// 注册/删除事件的核心操作 调用 epoll_ctl() 函数
// EPOLL_CTL_ADD 将文件描述符注册到epoll例程
// EPOLL_CTL_DEL 从epoll例程中删除文件描述符
// EPOLL_CTL_MOD 更改注册的文件描述符的关注事件发生情况
void EPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    // 通过将channel赋值给ptr，可以相互绑定，之后可以通过调用 events_[i].data.ptr来获取对应channel
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl";

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR << "epoll_ctl op = DEL fd = " << fd;
        }
        else
        {
            LOG_FATAL << "epoll_ctl op = "
                      << " fd = " << fd;
        }
    }
}