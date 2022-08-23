#pragma once

#include "muduo/base/noncopyable.h"

#include <map>
#include <vector>

struct epoll_event;

namespace muduo
{
    namespace net
    {
        class Channel;

        ///
        /// IO Multiplexing with epoll(4).
        ///
        /// This class doesn't own the Channel objects.
        class EPoller : noncopyable
        {
        public:
            typedef std::vector<Channel*> ChannelList;

            EPoller();
            ~EPoller();

            void epoll(int timeoutMs, ChannelList* activeChannels);

            void updataChannel(Channel* channel);

        private:
            typedef std::map<int, Channel *> ChannelMap;
            typedef std::vector<struct epoll_event> EventList;

            static const int kInitEventListSize = 16;

            void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

            void update(int operation, Channel *channel);

            ChannelMap channels_; // 存储 channel 的 map
            int epollfd_;         // epoll_create() 创建成功返回的文件描述符
            EventList events_;    // 传递给 epoll_wait() 时 发生变化的文件描述符信息将被填入该数组
        };

    } // namespace net

} // namespace muduo
