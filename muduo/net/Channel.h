#pragma once

#include "muduo/base/noncopyable.h"

#include <functional>
#include <memory>

///
/// Channel是 selectable IO channel，负责注册与响应IO事件
/// 它不拥有file descriptor
///

namespace muduo
{
    namespace net
    {
        /// The file descriptor could be a socket,
        /// an eventfd, a timerfd, or a signalfd
        class Channel : noncopyable
        {
        public:
            typedef std::function<void()> EventCallback;

            Channel(int fd);
            ~Channel();

            void handleEvent();

            void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }

            int fd() const { return fd_; }
            int events() const { return events_; }
            void set_revents(int revt) { revents_ = revt; }

            void enableReading()
            {
                events_ |= kReadEvent;
                // update();
            } // 注册可读事件

        private:
            // void update(); //FIXME 应该交由Epoller调用 这里先用TcpServer调用

            static const int kNoneEvent;  // 无事件
            static const int kReadEvent;  // 可读事件
            static const int kWriteEvent; // 可写事件

            const int fd_; // channel负责的文件描述符
            int events_;   // 要注册的IO事件
            int revents_;  // 目前活动的事件，由 Epoll 返回
            EventCallback readCallback_;
        };

    } // namespace net

} // namespace muduo
