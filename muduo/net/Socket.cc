#include "muduo/net/Socket.h"
#include "muduo/base/Logging.h"
#include "muduo/net/Address.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> // snprintf

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Socket::~Socket()
{
    if (::close(sockfd_) < 0)
    {
        LOG_ERROR << "Socket::~Socket close";
    }
}

bool Socket::getTcpInfo(struct tcp_info *tcpi) const
{
    socklen_t len = sizeof(*tcpi);
    memset(tcpi, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char *buf, int len) const
{
    // TODO
    return true;
}

void Socket::bindAddress(const IPv4Address &addr)
{
    int ret = ::bind(sockfd_, addr.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if (ret < 0)
    {
        LOG_FATAL << "Socket::bindAddress";
    }
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0)
    {
        LOG_FATAL << "Socket::listen";
    }
}

int Socket::accept(IPv4Address *peeraddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(&addr));
    int connfd = ::accept(sockfd_, reinterpret_cast<sockaddr *>(&addr), &addrlen);
                        //    SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet(addr);
    }
    else
    {
        LOG_ERROR << "Socket::accept";
        exit(1);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR << "Socket::shutdownWrite";
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
    // FIXME CHECK
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
    // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0 && on)
    {
        LOG_ERROR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
    {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
    // FIXME CHECK
}