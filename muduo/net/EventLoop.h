#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <chrono>

#include "muduo/base/Mutex.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/TimerId.h"

using namespace std::chrono;

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
            ~EventLoop(); // force out-line dtor, for std::uniqur_ptr members

            ///
            /// Loops forever.
            ///
            /// Must be called in the same thread as creation of the object.
            ///
            void loop();

            /// Quits loop.
            ///
            /// This is not 100% thread safe, if you call through a raw pointer,
            /// better to call through shared_ptr<EventLoop> for 100% safety.
            void quit();

            ///
            /// Time when epoll returns, usually means data arrival.
            ///
            system_clock::time_point epollReturnTime() const { return epollReturnTime_; }

            int64_t iteration() const { return iteration_; }

            /// Runs callback immediately in the loop thread.
            /// It wakes up the loop, and run the cb.
            /// If in the same loop thread, cb is run within the function.
            /// Safe to call from other threads.
            void runInLoop(Functor cb);
            /// Queues callback in the loop thread.
            /// Runs after finish pooling.
            /// Safe to call from other threads.
            void queueInLoop(Functor cb);

            size_t queueSize() const;

            // timers

            ///
            /// Runs callback at 'time'.
            /// Safe to call from other threads.
            ///
            TimerId runAt(system_clock::time_point time, TimerCallback cb);
            ///
            /// Runs callback after @c delay seconds.
            /// Safe to call from other threads.
            ///
            TimerId runAfter(double delay, TimerCallback cb);
            ///
            /// Runs callback every @c interval seconds.
            /// Safe to call from other threads.
            ///
            TimerId runEvery(double interval, TimerCallback cb);
            ///
            /// Cancels the timer.
            /// Safe to call from other threads.
            ///
            void cancel(TimerId timerId);

            // internal usage
            void wakeup();
            void updateChannel(Channel *channel);
            void removeChannel(Channel *channel);
            bool hasChannel(Channel *channel);

            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
            // bool callingPendingFunctors() const { return callingPendingFunctors_; }
            bool eventHandling() const { return eventHandling_; }

            static EventLoop *getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();
            void handleRead();        // waked up 将eventfd里的内容读走，以便让其检测事件通知
            void doPendingFunctors(); // 执行转交给IO的任务

            void printActiveChannels() const;   // DEBUG 将发生的事件写入日志

            typedef std::vector<Channel *> ChannelList; // EventLoop 管理的事件分发器列表

            bool looping_; // atomic
            std::atomic<bool> quit_;
            bool eventHandling_; // atomic
            bool callingPendingFunctors_; // atomic
            int64_t iteration_;
            const pid_t threadId_;
            system_clock::time_point epollReturnTime_;
            std::unique_ptr<EPoller> epoller_;
            std::unique_ptr<TimerQueue> timerQueue_; // 定时器列表
            int wakeupFd_;                             // eventfd 描述符， 用于唤醒阻塞在 epoll 的 IO 线程
            // unlike in TimerQueue, which is an internal class,
            // we don't expose Channel to client.
            std::unique_ptr<Channel> wakeupChannel_; // eventfd 对应的 Channel

            // scratch variables
            ChannelList activeChannels_;    // 活跃的事件列表
            Channel *currentActiveChannel_; // 当前要处理的事件

            mutable MutexLock mutex_;
            std::vector<Functor> pendingFunctors_; // 需要在主IO线程执行的任务
        };

    } // namespace net

} // namespace muduo
