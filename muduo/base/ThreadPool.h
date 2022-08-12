#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

#include <memory>
#include <functional>
#include <deque>
#include <vector>
#include <string>

// 线程池实现
// 管理一个任务队列，一个线程队列，然后每次取一个任务
// 分配给一个线程去做，循环往复

namespace muduo
{

    class ThreadPool : noncopyable
    {
    public:
        typedef std::function<void ()> Task;

        explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
        ~ThreadPool();

        // Must be called before start().
        void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
        void setThreadInitCallback(const Task& cb)
        {
            threadInitCallback_ = cb;
        }

        void start(int numThreads);
        void stop();

        const std::string& name() const { return name_; }

        size_t queueSize() const;

        void run(Task f);

    private:
        bool isFull() const;
        void runInThread();
        Task take();

        mutable MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        std::string name_;
        Task threadInitCallback_;
        std::vector<std::unique_ptr<muduo::Thread>> threads_;
        std::deque<Task> queue_;
        size_t maxQueueSize_;
        bool running_;
    };

}   // namespace muduo