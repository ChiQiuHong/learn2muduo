/**
* @description: main.cc
* @brief: 丢弃所有收到的数据
* @date: 2022/08/09 14:02:04
*/

#include "examples/simple/discard/discard.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace muduo;
using namespace muduo::net;

/**
 * @brief 不使用muduo库的写法
 */
// void DiscardServer() {
//     int serv_sock;
//     int clnt_sock;

//     struct sockaddr_in serv_addr;
//     struct sockaddr_in clnt_addr;
//     socklen_t clnt_addr_size;

//     char message[1024];

//     serv_sock = socket(PF_INET, SOCK_STREAM, 0);
//     if (serv_sock == -1)
//         exit(1);

//     memset(&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     serv_addr.sin_port = htons(2012);

//     if (bind(serv_sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1)
//         exit(1);

//     if (listen(serv_sock, 5) == -1)
//         exit(1);

//     clnt_addr_size = sizeof(clnt_addr);
//     clnt_sock = accept(serv_sock, reinterpret_cast<struct sockaddr*>(&clnt_addr), &clnt_addr_size);
//     if (clnt_sock == -1)
//         exit(1);
    
//     read(clnt_sock, message, sizeof(message) - 1);
//     printf("Message from client: %s \n", message);
//     close(serv_sock);
//     close(clnt_sock);
// }

/**
 * @brief 使用muduo库的写法
 */

int main() {
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    IPv4Address listenAddr(2022);
    DiscardServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}