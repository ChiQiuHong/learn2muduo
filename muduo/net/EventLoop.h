#pragma once

#include <atomic>
#include <functional>
#include <vector>

#include "muduo/base/Mutex.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/net/Callbacks.h"

///
/// EventLoop 事件循环（反应器Reactor） 每个线程只能有一个EventLoop实体
/// 它负责IO和定时器事件的分派，它用eventfd(2) 来异步唤醒
/// 这有别于传统的用一对pipe(2) 的办法
/// 用 TimerQueue 作为计时器管理
/// 用 EPoller 作为IO multiplexing

namespace muduo
{
    namespace net
    {

        class Channel;
        class EPoller;
        class TimerQueue;

        ///
        /// Reactor, at most one per thread.
        ///
        /// This is an interface class, so don't expose much details.
        class EventLoop : noncopyable
        {
        public:
            typedef std::function<void()> Functor;

            EventLoop();
            ~EventLoop();   // force out-line dtor, for std::uniqur_ptr members

            ///
            /// Loops forever.
            ///
            /// Must be called in the same thread as creation of the object.
            /// 
            void loop();

            /// Runs callback immediately in the loop thread.
            /// It wakes up the loop, and run the cb.
            /// If in the same loop thread, cb is run within the function.
            /// Safe to call from other threads.
            void runInLoop(Functor cb);

            void updateChannel(Channel* channel);
            // void removeChannel(Channel* channel);

            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

            static EventLoop* getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();    

            // void printActiveChannels() const;
            
            typedef std::vector<Channel*> ChannelList;

            bool looping_;  // atomic
            const pid_t threadId_;
            std::unique_ptr<EPoller> epoller_;
            std::unique_ptr<TimerQueue> timerQueue_;

            ChannelList activeChannels_;    // 活跃的事件列表
            Channel* currentActiveChannel_; // 当前要处理的事件
        };
        
    } // namespace net
    
} // namespace muduo
