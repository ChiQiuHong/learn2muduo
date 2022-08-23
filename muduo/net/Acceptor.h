#pragma once

#include "muduo/net/Channel.h"
#include "muduo/net/Socket.h"

#include <functional>

///
/// Acceptor用于接受TCP连接，它是TcpServer的成员，生命期由后者控制
///

namespace muduo
{
    namespace net
    {
        class EventLoop;
        class IPv4Address;

        ///
        /// Acceptor of incoming TCP connections.
        ///
        class Acceptor : noncopyable
        {
        public:
            typedef std::function<void (int sockfd, const IPv4Address&)> NewConnectionCallback;

            Acceptor(EventLoop* loop, const IPv4Address& listenAddr, bool reuseport);
            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

            void listen();
            bool listening() const { return listening_; }

        private:
            void handleRead();  // 调用accept4()来接受新连接，并执行用户传递的newConnectionCallback_

            EventLoop* loop_;
            Socket acceptSocket_;   // listening socket 即 server socket
            Channel acceptChannel_; // 响应此 socket 上的 readable 事件 若有新连接则执行回调函数 Acceptor::handleRead()
            NewConnectionCallback newConnectionCallback_;
            bool listening_;
            int idleFd_;    // idle为空闲的意思，用作占位的空闲描述符，见 muduo 7.7 节
        };
        
    } // namespace net
    
} // namespace muduo
