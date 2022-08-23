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
        class EventLoop;

        /// The file descriptor could be a socket,
        /// an eventfd, a timerfd, or a signalfd
        class Channel : noncopyable
        {
        public:
            typedef std::function<void()> EventCallback;

            Channel(EventLoop* loop, int fd);
            ~Channel();

            void handleEvent();

            void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }

            // 返回 Channel 负责的文件描述符
            int fd() const { return fd_; }
            // 返回 Channel 给文件描述符设置的监听事件
            int events() const { return events_; }
            // 由EPoller调用 将 活动事件 保存在 revents 中
            void set_revents(int revt) { revents_ = revt; }

            // 注册可读事件
            void enableReading()
            {
                events_ |= kReadEvent;
                update();
            }

            EventLoop* ownerLoop() { return loop_; }

        private:
            void update();

            static const int kNoneEvent;  // 无事件
            static const int kReadEvent;  // 可读事件
            static const int kWriteEvent; // 可写事件

            EventLoop *loop_; // channel 所属的loop
            const int fd_;    // channel负责的文件描述符
            int events_;      // 要注册的IO事件
            int revents_;     // 目前活动的事件，由 Epoll 返回
            EventCallback readCallback_;
        };

    } // namespace net

} // namespace muduo
