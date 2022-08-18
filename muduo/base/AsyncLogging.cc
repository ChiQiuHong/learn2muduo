#include "muduo/base/AsyncLogging.h"
#include "muduo/base/LogFile.h"

#include <chrono>

using namespace std::chrono;

namespace muduo
{
    AsyncLogging::AsyncLogging(const std::string &basename,
                               off_t rollSize,
                               int flushInterval)
        : flushInterval_(flushInterval),
          running_(false),
          basename_(basename),
          rollSize_(rollSize),
          thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
          latch_(1),
          mutex_(),
          cond_(mutex_),
          currentBuffer_(new Buffer),
          nextBuffer_(new Buffer),
          buffers_()
    {
        currentBuffer_->bzero();
        nextBuffer_->bzero();
        buffers_.reserve(16);
    }

    void AsyncLogging::append(const char *logline, int len)
    {
        muduo::MutexLockGuard lock(mutex_);
        if (currentBuffer_->avail() > len)
        {
            currentBuffer_->append(logline, len);
        }
        else
        {
            buffers_.push_back(std::move(currentBuffer_));

            if (nextBuffer_)
            {
                currentBuffer_ = std::move(nextBuffer_);
            }
            else
            {
                currentBuffer_.reset(new Buffer); // Rarely happens
            }
            currentBuffer_->append(logline, len);
            cond_.notify();
        }
    }

    void AsyncLogging::threadFunc()
    {   
        assert(running_ == true);
        latch_.countDown();
        LogFile output(basename_, rollSize_, false);
        BufferPtr newBuffer1(new Buffer);
        BufferPtr newBuffer2(new Buffer);
        newBuffer1->bzero();
        newBuffer2->bzero();
        BufferVector buffersToWrite;
        buffersToWrite.reserve(16);
        while (running_)
        {
            {
                muduo::MutexLockGuard lock(mutex_);
                if (buffers_.empty())   // unusual usage!
                {
                    
                }
                buffers_.push_back(std::move(currentBuffer_));
                currentBuffer_ = std::move(newBuffer1);
                buffersToWrite.swap(buffers_);
                if (!nextBuffer_)
                {
                    nextBuffer_ = std::move(newBuffer2);
                }
            }
            assert(!buffersToWrite.empty());

            buffersToWrite.clear();
            output.flush();
        }
        output.flush();
    }

} // namespace muduo
