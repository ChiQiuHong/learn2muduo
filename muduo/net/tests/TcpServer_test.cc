#include "muduo/net/TcpServer.h"
#include "muduo/base/Logging.h"

#include <string.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void DiscardServer() {

    IPv4Address serv_addr(2032, true);

    TcpServer server(serv_addr, "DiscardServer");
    printf("this is server: %s\n", serv_addr.toIpPort().c_str());
    server.start();
    
    char message[1024];
    server.handleRead(message);
    printf("Message from client: %s \n", message);
}

int main()
{
    DiscardServer();
}