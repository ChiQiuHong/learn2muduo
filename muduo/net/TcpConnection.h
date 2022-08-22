#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/net/Address.h"
#include "muduo/net/Socket.h"

///
/// TcpConection 整个网络库的核心，封装一次TCP连接，注意它不能发起连接
///

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace muduo
{
    namespace net
    {
        ///
        /// Tcp connection, for both client and server usage.
        ///
        /// This is an interface class, so don't expose too much details.
        class TcpConnection : noncopyable
        {
        public:
            TcpConnection(int sockfd);
            ~TcpConnection();

            void handleRead(char* buf);

        private:

            Socket socket_;
        };

    } // namespace net
    
} // namespace muduo
