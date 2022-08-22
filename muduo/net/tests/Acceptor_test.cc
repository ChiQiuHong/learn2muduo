#include "muduo/net/Acceptor.h"
#include "muduo/net/Address.h"
#include "muduo/net/Socket.h"
#include "muduo/base/Logging.h"

#include <string.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void Read(int connfd, const IPv4Address& peerAddr)
{
    char message[1024];
    ::read(connfd, message, sizeof(message) - 1);
    printf("Message from client: %s : %s\n", peerAddr.toIpPort().c_str(), message);
}

void DiscardServer() {

    IPv4Address serv_addr(2020, true);

    Acceptor server(serv_addr, true);
    server.setNewConnectionCallback(Read);
    printf("this is server: %s\n", serv_addr.toIpPort().c_str());
    server.listen();
    server.handleRead();
}

int main()
{
    DiscardServer();
}