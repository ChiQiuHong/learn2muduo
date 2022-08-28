#include "muduo/net/EPoller.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"

#include <sys/epoll.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
    const int kNew = -1;    // 新增
    const int kAdded = 1;   // 已添加
    const int kDeleted = 2; // 已删除
}

EPoller::EPoller(EventLoop *loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL << "EPoller::EPoller"; // FIXME LOG_SYSFATAL
    }
}

EPoller::~EPoller()
{
    ::close(epollfd_);
}

system_clock::time_point EPoller::epoll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_TRACE << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    auto now = system_clock::now();
    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
        // 调整events_数组的空间
        if (static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_TRACE << "nothing happened";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_ERROR << "EPoller::epoll()"; // FIXME: LOG_SYSERR
        }
    }

    return now;
}

/// 添加新Channel的复杂度是O(log N)
/// 更新已有的Channel的复杂度是O(1)
void EPoller::updataChannel(Channel *channel)
{
    EPoller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd()
              << " events = " << channel->events() << " index = " << index;
    // 如果 channel 是新增的或是已经从epoll里删除了的
    if (index == kNew || index == kDeleted)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // index == kDeleted
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        // channel 设为 kAdded 状态 将 fd 注册到 epoll 例程
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent())
        {
            // 如果 Channle 暂时不关心任何事件了 将 fd 从 epoll 里删除
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else    // 否则 修改已经注册的 fd 的事件 如可读或可写
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 移除Channel 时间复杂度O(log N)
void EPoller::removeChannel(Channel *channel)
{
    EPoller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

bool EPoller::hasChannel(Channel *channel) const
{
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

const char *EPoller::operationToString(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}

void EPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
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
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString() << " }";

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