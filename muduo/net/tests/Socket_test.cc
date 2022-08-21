#include "muduo/net/Address.h"
#include "muduo/net/Socket.h"
#include "muduo/base/Logging.h"

#include <string.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void DiscardServer() {

    IPv4Address serv_addr(2030, true);
    IPv4Address clnt_addr;

    char message[1024];

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        LOG_ERROR << "::socket failed";
        exit(1);
    }
    Socket serv_sock(sockfd);

    serv_sock.bindAddress(serv_addr);
    printf("this is server: %s\n", serv_addr.toIpPort().c_str());
    serv_sock.listen();

    int clnt_sock = serv_sock.accept(&clnt_addr);
    
    read(clnt_sock, message, sizeof(message) - 1);
    printf("Message from client %s: %s \n", clnt_addr.toIpPort().c_str(), message);

    close(clnt_sock);
}

int main()
{
    DiscardServer();
}