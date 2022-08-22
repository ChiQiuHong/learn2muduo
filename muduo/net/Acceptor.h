#pragma once

#include "muduo/net/Socket.h"

///
/// Acceptor用于接受TCP连接，它是TcpServer的成员，生命期由后者控制
///

namespace muduo
{
    namespace net
    {
        class IPv4Address;

        ///
        /// Acceptor of incoming TCP connections.
        ///
        class Acceptor : noncopyable
        {
        public:
            Acceptor(const IPv4Address& listenAddr, bool reuseport);
            ~Acceptor();

            void listen();
            void handleRead(char* buf);

        private:
            Socket acceptSocket_;
        };
        
    } // namespace net
    
} // namespace muduo
