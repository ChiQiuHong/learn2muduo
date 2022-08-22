#include "muduo/net/TcpServer.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Acceptor.h"

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(const IPv4Address& listenAddr,
                     const std::string& nameArg)
    : name_(nameArg),
      acceptor_(new Acceptor(listenAddr, true))
{

}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
    acceptor_->listen();
}

void TcpServer::handleRead(char* buf)
{
    acceptor_->handleRead(buf);
}