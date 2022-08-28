#pragma once

#include "muduo/net/TcpServer.h"

class DiscardServer
{
public:
    DiscardServer(muduo::net::EventLoop* loop,
                  const muduo::net::IPv4Address& listenAddr);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   system_clock::time_point time);
    
    muduo::net::TcpServer server_;

};