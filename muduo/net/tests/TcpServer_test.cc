#include "muduo/net/TcpServer.h"
#include "muduo/base/Logging.h"

#include <string.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void onMessage(const TcpConnectionPtr &conn)
{
    char message[1024];
    ::read(conn->fd(), message, sizeof(message) - 1);
    printf("Message from client: %s: %s \n", conn->peerAddress().toIpPort().c_str(), message);
}

void DiscardServer()
{

    IPv4Address serv_addr(2030, true);

    TcpServer server(serv_addr, "DiscardServer");
    server.setMessageCallback(onMessage);
    server.start();
    server.handleRead();
}

int main()
{
    DiscardServer();
}