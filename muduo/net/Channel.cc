#include "muduo/net/Channel.h"

#include "muduo/base/Logging.h"

#include <sys/epoll.h>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(int fd__)
    : fd_(fd__),
      revents_(0)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent()
{
    // 发生可读事件
    if (revents_ & EPOLLIN)
    {
        if (readCallback_)
        {
            // 执行用户注册的回调函数
            readCallback_();
        }
    }
}

// void Channel::update()
// {

// }