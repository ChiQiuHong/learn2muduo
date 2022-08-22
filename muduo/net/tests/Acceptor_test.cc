#include "muduo/net/Acceptor.h"
#include "muduo/net/Address.h"
#include "muduo/net/Socket.h"
#include "muduo/base/Logging.h"

#include <string.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void DiscardServer() {

    IPv4Address serv_addr(2031, true);

    Acceptor server(serv_addr, true);
    printf("this is server: %s\n", serv_addr.toIpPort().c_str());
    server.listen();
    
    char message[1024];
    server.handleRead(message);
    printf("Message from client: %s \n", message);
}

int main()
{
    DiscardServer();
}