#pragma once

#include "muduo/base/noncopyable.h"

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace muduo
{

    ///
    /// TCP networking
    ///
    namespace net
    {
        class IPv4Address;

        ///
        /// Wrapper of socket file descriptor.
        ///
        /// It closes the sockfd when desctructs.
        /// It's thread saft, all operations are delagated to OS.
        class Socket : noncopyable
        {
        public:
            explicit Socket(int sockfd)
                : sockfd_(sockfd)
            {
            }

            ~Socket();

            int fd() const { return sockfd_; }
            // return true if success.
            bool getTcpInfo(struct tcp_info *) const;
            bool getTcpInfoString(char *buf, int len) const;

            // abort if address in use
            void bindAddress(const IPv4Address &localaddr);
            // abort if address in use
            void listen();

            // On success, returns a non-negative integer that is
            // a descriptor for the accetped socket, which has been
            // set to non-blocking and close-on-exec. *peeraddr is assigned.
            // On error, -1 is returned, and * peeraddr is untouched.
            int accept(IPv4Address *peeraddr);

            // 半关闭写
            void shutdownWrite();

            // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
            // 设置是否 Nagel 算法，默认不使用
            void setTcpNoDelay(bool on);

            // Enable/disable SO_REUSEADDR
            // 设置 Time-wait 状态下是否重新分配新的套接字 默认重新分配
            void setReuseAddr(bool on);

            // Enable/disable SO_REUSEPORT
            // 设置是否将多个socket绑定在同一个监听断开 默认开启
            void setReusePort(bool on);

            // Enable/disable SO_KEEPALIVE
            // 是否开启TCP保活机制 默认开启
            void setKeepAlive(bool on);

        private:
            const int sockfd_;
        };

    } // namespace net

} // namespace muduo
