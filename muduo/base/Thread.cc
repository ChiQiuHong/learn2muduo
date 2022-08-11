#include "muduo/base/Thread.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <assert.h>

#include <thread>

namespace muduo
{

    namespace detail
    {
        pid_t gettid()
        {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }

        struct ThreadData
        {
            typedef muduo::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            std::string name_;
            pid_t *tid_;

            ThreadData(ThreadFunc func,
                       const std::string &name,
                       pid_t *tid)
                : func_(std::move(func)),
                  name_(name),
                  tid_(tid)
            {
            }

            void runInThread()
            {
                *tid_ = detail::gettid();
                printf("run in thread tid=%d\n", *tid_);    // FIXME
                tid_ = NULL;
                func_();
            }
        };

        void* startThread(void* obj)
        {
            ThreadData* data = static_cast<ThreadData*>(obj);
            data->runInThread();
            delete data;
            return NULL;
        }

    } // namespace detail

    Thread::Thread(ThreadFunc func, const std::string &n)
        : started_(false),
          joined_(false),
          pthreadId_(0),
          tid_(0),
          func_(std::move(func)),
          name_(n),
          numCreated_(0)
    {
        setDefaultName();
    }

    Thread::~Thread()
    {
        if (started_ && !joined_)
        {
            pthread_detach(pthreadId_);
        }
    }

    void Thread::setDefaultName()
    {
        int num = numCreated_ + 1;
        if (name_.empty())
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "Thread%d", num);
            name_ = buf;
        }
    }

    void Thread::start()
    {
        assert(!started_);
        started_ = true;
        // FIXME: move(func_)
        detail::ThreadData *data = new detail::ThreadData(func_, name_, &tid_);
        if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
        {
            started_ = false;
            delete data; // or no delete?
            printf("Failed in pthread_create");
        }
    }

    int Thread::join()
    {
        assert(started_);
        assert(!joined_);
        joined_ = true;
        return pthread_join(pthreadId_, NULL);
    }

} // namespace muduo