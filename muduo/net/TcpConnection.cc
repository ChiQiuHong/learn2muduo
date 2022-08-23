#include "muduo/net/TcpConnection.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Socket.h"

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void muduo::net::defaultConnectionCallback(const TcpConnectionPtr &conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void muduo::net::defaultMessageCallback(const TcpConnectionPtr &conn)
{
    char message[1024];
    ::read(conn->fd(), message, sizeof(message) - 1);
    printf("Message from client: %s: %s \n", conn->peerAddress().toIpPort().c_str(), message);
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &nameArg,
                             int sockfd,
                             const IPv4Address &localAddr,
                             const IPv4Address &peerAddr)
    : loop_(loop),
      name_(nameArg),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    LOG_DEBUG << "TcpConnection::ctor[" << name_.c_str() << "] at "
              << this << " fd=" << sockfd;
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_.c_str() << "] at " << this
              << " fd=" << socket_->fd();
}

void TcpConnection::connectEstablished()
{
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead()
{
    messageCallback_(shared_from_this());
}

int TcpConnection::fd() const
{
    return socket_->fd();
}