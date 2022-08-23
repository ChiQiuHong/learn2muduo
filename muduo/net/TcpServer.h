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
        class EventLoop;

        ///
        /// TCP server, supports single-threaded and thread-pool models.
        ///
        /// This is an interface class, so don't expose too much details.
        class TcpServer : noncopyable
        {
        public:

            enum Option { kNoReusePort, kReusePort };

            TcpServer(EventLoop* loop,
                      const IPv4Address &listenAddr,
                      const std::string &nameArg,
                      Option option = kNoReusePort);
            ~TcpServer();

            EventLoop* getLoop() const { return loop_; }

            void start();
            
            void setConnectionCallback(const ConnectionCallback& cb)
            {
                connectionCallback_ = cb;
            }

            void setMessageCallback(const MessageCallback& cb)
            {
                messageCallback_ = cb;
            }

        private:
            void newConnection(int sockfd, const IPv4Address &peerAddr);

            typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

            EventLoop* loop_;
            const std::string ipPort_;
            const std::string name_;
            std::unique_ptr<Acceptor> acceptor_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            int nextConnId_;
            ConnectionMap connections_;
        };

    } // namespace net

} // namespace muduo
