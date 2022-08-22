#pragma once

#include "muduo/base/Atomic.h"
#include "muduo/net/TcpConnection.h"

#include <map>
#include <memory>

///
/// 用于编写网络服务器，接受客户的连接
///

namespace muduo
{
    namespace net
    {

        class Acceptor;

        ///
        /// TCP server, supports single-threaded and thread-pool models.
        ///
        /// This is an interface class, so don't expose too much details.
        class TcpServer : noncopyable
        {
        public:
            TcpServer(const IPv4Address &listenAddr,
                      const std::string &nameArg);
            ~TcpServer();

            void setMessageCallback(const MessageCallback& cb)
            {
                messageCallback_ = cb;
            }

            void start();

            void handleRead();

        private:
            void newConnection(int sockfd, const IPv4Address &peerAddr);

            typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

            const std::string ipPort_;
            const std::string name_;
            std::unique_ptr<Acceptor> acceptor_;
            MessageCallback messageCallback_;
            int nextConnId_;
            ConnectionMap connections_;
        };

    } // namespace net

} // namespace muduo
