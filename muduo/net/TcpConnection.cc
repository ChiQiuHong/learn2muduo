#include "muduo/net/TcpConnection.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Socket.h"

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

TcpConnection::TcpConnection(const std::string &nameArg,
                             int sockfd,
                             const IPv4Address &localAddr,
                             const IPv4Address &peerAddr)
    : name_(nameArg),
      socket_(new Socket(sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
    LOG_DEBUG << "TcpConnection::ctor[" << name_.c_str() << "] at "
              << this << " fd=" << sockfd;
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_.c_str() << "] at " << this
              << " fd=" << socket_->fd();
}

int TcpConnection::fd() const
{
    return socket_->fd();
}

void TcpConnection::handleRead()
{
    messageCallback_(shared_from_this());
}