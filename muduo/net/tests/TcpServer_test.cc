#include "muduo/net/TcpServer.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <string.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "DiscardServer - " << conn->peerAddress().toIpPort().c_str() << " -> "
             << conn->localAddress().toIpPort().c_str() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr &conn,
               Buffer* buf,
               system_clock::time_point time)
{
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " discards " << msg.size()
             << " bytes reveived at ";
}

void DiscardServer()
{

    IPv4Address serv_addr(2035, true);
    EventLoop loop;

    TcpServer server(&loop, serv_addr, "DiscardServer");
    server.setMessageCallback(onMessage);
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();
}

int main()
{    
    DiscardServer();
}