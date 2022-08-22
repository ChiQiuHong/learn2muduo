#pragma once

#include "muduo/net/Socket.h"

#include <functional>

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
            typedef std::function<void (int sockfd, const IPv4Address&)> NewConnectionCallback;

            Acceptor(const IPv4Address& listenAddr, bool reuseport);
            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

            void listen();
            void handleRead();

        private:
            Socket acceptSocket_;
            NewConnectionCallback newConnectionCallback_;
        };
        
    } // namespace net
    
} // namespace muduo
