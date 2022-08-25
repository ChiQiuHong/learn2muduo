#include "examples/simple/discard/discard.h"

#include "muduo/base/Logging.h"

using namespace muduo;
using namespace muduo::net;

DiscardServer::DiscardServer(EventLoop* loop,
                             const IPv4Address& listenAddr)
    : server_(loop, listenAddr, "DiscardServer")
{
    server_.setConnectionCallback(
        std::bind(&DiscardServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&DiscardServer::onMessage, this, _1, _2, _3));
}

void DiscardServer::start()
{
    server_.start();
}

void DiscardServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "DiscardServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}

void DiscardServer::onMessage(const TcpConnectionPtr& conn,
                              system_clock::timepoint time)
{
    LOG_INFO << conn->name() << " discards " << msg.size()
             << " bytes received at " << ;
}