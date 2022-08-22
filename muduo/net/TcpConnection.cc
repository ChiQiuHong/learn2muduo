#include "muduo/net/TcpConnection.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Socket.h"

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

TcpConnection::TcpConnection(int sockfd)
    : socket_(sockfd)
{

}

TcpConnection::~TcpConnection()
{
    
}

void TcpConnection::handleRead(char* buf)
{
    ::read(socket_.fd(), buf, sizeof(buf) - 1);
}