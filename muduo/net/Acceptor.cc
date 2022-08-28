#include "muduo/net/Acceptor.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Address.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Socket.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop, const IPv4Address &listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(::socket(listenAddr.family(), SOCK_STREAM, IPPROTO_TCP)),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false),
      idleFd_(1) // TODO 占位符 ::open("/dev/null", O_RDONLY | O_CLOEXEC)
{
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    // 连接关闭 要关闭channel 移除注册的epoll
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    // ::close(idleFd_);
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    // epoller注册可读事件 当有可读事件发生就会调用 Acceptor::handleRead
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    IPv4Address peeraddr;
    // FIXME loop until no more
    int connfd = acceptSocket_.accept(&peeraddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peeraddr);
        }
        // else
        // {
        //     // TODO
        //     // sockets::close(connfd);
        // }
    }
    // 如果connfd小于0，就是说明文件描述符耗尽了，这时候我们关闭预留的dileFd_
    // 那么就会有一个空闲的文件描述符空出来，我们立即去接受新连接，然后立即关闭
    // 重新占用这个空闲的文件描述符。
    else
    {
        // TODO
        LOG_ERROR << "in Acceptor::handleRead"; // FIXME LOG_SYSERR
        if (errno == EMFILE)
        {
            LOG_ERROR << "errno EMFILE";
            // if(errno == EMFILE)
            // {
            //     ::close(idleFd_);
            //     idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            //     ::close(idleFd_);
            //     idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            // }
        }
    }
}