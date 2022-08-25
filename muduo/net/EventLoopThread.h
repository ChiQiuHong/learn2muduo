#pragma once

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

#include <string>

namespace muduo
{
    namespace net
    {
        class EventLoop;

        class EventLoopThread : noncopyable
        {
        public:
            typedef std::function<void(EventLoop *)> ThreadInitCallback;

            EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                            const std::string &name = std::string());
            ~EventLoopThread();

            EventLoop *startLoop();

        private:
            void threadFunc();

            EventLoop *loop_;
            bool exiting_;
            Thread thread_;
            MutexLock mutex_;
            Condition cond_;              // 用于保证初始化成功
            ThreadInitCallback callback_; // 线程初始化的回调函数
        };
    } // namespace net

} // namespace muduo
