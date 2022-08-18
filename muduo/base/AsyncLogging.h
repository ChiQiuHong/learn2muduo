#pragma once

#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Condition.h"
#include "muduo/base/LogStream.h"

#include <atomic>
#include <vector>
#include <string>

namespace muduo
{
    class AsyncLogging : noncopyable
    {
    public:
        AsyncLogging(const std::string& basename,
                     off_t rollSize,
                     int flushInterval = 3);

        ~AsyncLogging()
        {
            if (running_)
            {
                stop();
            }
        }

        void append(const char *logline, int len);

        void start()
        {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        void stop()
        {
            running_ = false;
            cond_.notify();
            thread_.join();
        }

    private:

        void threadFunc();

        typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
        typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
        typedef BufferVector::value_type BufferPtr;

        const int flushInterval_;
        std::atomic<bool> running_;
        const std::string basename_;
        const off_t rollSize_;
        muduo::Thread thread_;
        muduo::CountDownLatch latch_;
        muduo::MutexLock mutex_;
        muduo::Condition cond_;
        BufferPtr currentBuffer_; // 当前缓冲
        BufferPtr nextBuffer_;    // 预备缓冲
        BufferVector buffers_;    // 待写入文件的已填满的缓冲
    };

} // namespace muduo