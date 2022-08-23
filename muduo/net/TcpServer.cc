#include "muduo/net/TcpServer.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Acceptor.h"
#include "muduo/net/EPoller.h"

using namespace muduo;
using namespace muduo::net;

namespace
{
    const int kEPollTimeMs = 10000;
}

TcpServer::TcpServer(const IPv4Address &listenAddr,
                     const std::string &nameArg)
    : ipPort_(listenAddr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(listenAddr, true)),
      nextConnId_(1),
      epoller_(new EPoller())
{
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
}

void TcpServer::start()
{
    acceptor_->listen();
    // 注册可读事件
    epoller_->updataChannel(acceptor_->getChannel());

    std::vector<Channel*> activeChannels;
    while (1)
    {
        activeChannels.clear();
        epoller_->epoll(kEPollTimeMs, &activeChannels);

        for (Channel* channel : activeChannels)
        {
            channel->handleEvent();
        }
    }
}

void TcpServer::newConnection(int sockfd, const IPv4Address &peerAddr)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName.c_str()
             << "] from " << peerAddr.toIpPort().c_str();

    // TODO 将下面这段移到socketops里
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    ::getsockname(sockfd, reinterpret_cast<sockaddr *>(&localaddr), &addrlen);
    IPv4Address localAddr(localaddr);

    TcpConnectionPtr conn(new TcpConnection(connName, sockfd, localAddr, peerAddr));

    connections_[connName] = conn;
    conn->setMessageCallback(messageCallback_);
    conn->connectEstablished();
    epoller_->updataChannel(conn->getChannel());
}