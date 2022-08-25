#pragma once

#include "muduo/base/noncopyable.h"

#include <functional>
#include <memory>
#include <chrono>
#include <string>

using namespace std::chrono;

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
            typedef std::function<void(system_clock::time_point)> ReadEventCallback;

            Channel(EventLoop *loop, int fd);
            ~Channel();

            void handleEvent(system_clock::time_point reveiveTime);

            void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
            void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
            void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
            void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

            /// Tie this channel to the owner object managed by shared_ptr,
            /// prevent the owner object being destroyed in handleEvent.
            void tie(const std::shared_ptr<void> &);

            int fd() const { return fd_; }
            int events() const { return events_; }
            // 由EPoller调用 将 活动事件 保存在 revents 中
            void set_revents(int revt) { revents_ = revt; } // used by epoller
            // int revents() const { return revents_; }
            bool isNoneEvent() const { return events_ == kNoneEvent; }

            // 注册可读事件
            void enableReading()
            {
                events_ |= kReadEvent;
                update();
            }

            void disableReading()
            {
                events_ &= ~kReadEvent;
                update();
            }

            void enableWriting()
            {
                events_ |= kWriteEvent;
                update();
            }

            void disableWriting()
            {
                events_ &= ~kWriteEvent;
                update();
            }

            void disableAll()
            {
                events_ = kNoneEvent;
                update();
            }

            bool isWriting() const { return events_ & kWriteEvent; }
            bool isReading() const { return events_ & kReadEvent; }

            // for Poller
            int index() { return index_; }
            void set_index(int idx) { index_ = idx; }

            // for debug
            std::string reventsToString() const;
            std::string eventsToString() const;

            void doNotLogHug() { logHup_ = false; }

            EventLoop *ownerLoop() { return loop_; }
            void remove();

        private:
            static std::string eventsToString(int fd, int ev);

            void update();
            void handleEventWithGuard(system_clock::time_point receiveTime);

            static const int kNoneEvent;  // 无事件
            static const int kReadEvent;  // 可读事件
            static const int kWriteEvent; // 可写事件

            EventLoop *loop_; // channel 所属的loop
            const int fd_;    // channel负责的文件描述符
            int events_;      // 要注册的IO事件
            int revents_;     // 目前活动的事件，由 Epoll 返回
            int index_;       // used by EPoll
            bool logHup_;     // 是否生成某些日志

            std::weak_ptr<void> tie_;
            bool tied_;
            bool eventHandling_;
            bool addedToLoop_;
            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback closeCallback_;
            EventCallback errorCallback_;
        };

    } // namespace net

} // namespace muduo
