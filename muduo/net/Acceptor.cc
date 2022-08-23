#include "muduo/net/Acceptor.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Address.h"
#include "muduo/net/Socket.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(const IPv4Address &listenAddr, bool reuseport)
    : acceptSocket_(::socket(listenAddr.family(), SOCK_STREAM, IPPROTO_TCP)),
      acceptChannel_(acceptSocket_.fd())
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
}

void Acceptor::listen()
{
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    IPv4Address peeraddr;

    int connfd = acceptSocket_.accept(&peeraddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peeraddr);
        }
    }
    else
    {
        LOG_ERROR << "in Acceptor::handleRead";
        if (errno == EMFILE)
        {
            LOG_ERROR << "errno EMFILE";
        }
    }
}