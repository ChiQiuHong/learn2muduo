#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/net/Address.h"
#include "muduo/net/Callbacks.h"

#include <memory>
#include <functional>

///
/// TcpConection 整个网络库的核心，封装一次TCP连接，注意它不能发起连接
///

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace muduo
{
    namespace net
    {
        class Channel;
        class EventLoop;
        class Socket;

        ///
        /// Tcp connection, for both client and server usage.
        ///
        /// This is an interface class, so don't expose too much details.
        class TcpConnection : noncopyable,
                              public std::enable_shared_from_this<TcpConnection>
        {
        public:
            TcpConnection(EventLoop* loop,
                          const std::string &name,
                          int sockfd,
                          const IPv4Address &localAddr,
                          const IPv4Address &peerAddr);
            ~TcpConnection();

            const IPv4Address& localAddress() const { return localAddr_; }
            const IPv4Address& peerAddress() const { return peerAddr_; }
            bool connected() const { return state_ == kConnected; }

            void setConnectionCallback(const ConnectionCallback& cb)
            {
                connectionCallback_ = cb;
            }

            void setMessageCallback(const MessageCallback &cb)
            {
                messageCallback_ = cb;
            }

            void connectEstablished();

            int fd() const;

        private:
            enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
            void handleRead();

            EventLoop* loop_;
            const std::string name_;
            StateE state_;
            std::unique_ptr<Socket> socket_;
            std::unique_ptr<Channel> channel_;
            const IPv4Address localAddr_;
            const IPv4Address peerAddr_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
        };

        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    } // namespace net

} // namespace muduo
