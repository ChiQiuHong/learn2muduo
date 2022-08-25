#pragma once

#include "muduo/net/TcpServer.h"

class DiscardServer
{
public:
    DiscardServer(muudo::net::EventLoop* loop,
                  const muduo::net::IPv4Address& listenAddr);

    void start();
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   system_clock::time_point time);
    
    muduo::net::TcpServer sersver_;

}