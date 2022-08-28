#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"

#include <functional>
#include <map>
#include <chrono>

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace muduo;
using namespace muduo::net;
using namespace std::chrono;

namespace muduo
{
    namespace net
    {
        namespace detail
        {
            int createTimerfd();
            void readTimerfd(int timerfd, system_clock::time_point now);
        } // namespace detail
        
    } // namespace net
    
} // namespace muduo

char buf[128];

char* formatTime(system_clock::time_point receiveTime)
{
    memset(buf, 0, sizeof(buf));
    time_t time = system_clock::to_time_t(receiveTime);
    struct tm tm_time;
    ::localtime_r(&time, &tm_time);

    size_t len = strftime(buf, sizeof(buf), "%Y%m%d-%H:%M:%S", &tm_time);
    auto us = duration_cast<microseconds>(receiveTime.time_since_epoch()) % 1000000;
    snprintf(buf+len, sizeof(buf), ".%06ldZ ", us.count());

    return buf;
}

double timeDifference(system_clock::time_point now, system_clock::time_point last)
{
    auto time_diff = now - last;
    auto duration = duration_cast<std::chrono::microseconds>(time_diff);
    return static_cast<double>(duration.count()) / 1000000; 
}

void print(const char* msg)
{
    static std::map<const char*, system_clock::time_point> lasts;
    system_clock::time_point& last = lasts[msg];
    system_clock::time_point now = system_clock::now();
    printf("%s tid %d %s delay %f\n", formatTime(now), CurrentThread::tid(), msg, timeDifference(now, last));
    last = now;
}

// Use ralative time, immunized to wall clock changes.
class PeriodicTimer
{
public:
    PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
        : loop_(loop),
          timerfd_(muduo::net::detail::createTimerfd()),
          timerfdChannel_(loop, timerfd_),
          interval_(interval),
          cb_(cb)
    {
        timerfdChannel_.setReadCallback(
            std::bind(&PeriodicTimer::handleRead, this));
        timerfdChannel_.enableReading();
    }

    void start()
    {
        struct itimerspec spec;
        memset(&spec, 0, sizeof(spec));
        spec.it_interval = toTimerSpec(interval_);
        spec.it_value = spec.it_interval;
        int ret = ::timerfd_settime(timerfd_, 0, &spec, NULL);
        if (ret)
        {
            LOG_ERROR << "timerfd_settime()";
        }
    }

    ~PeriodicTimer()
    {
        timerfdChannel_.disableAll();
        timerfdChannel_.remove();
        ::close(timerfd_);
    }

private:
    void handleRead()
    {
        loop_->assertInLoopThread();
        muduo::net::detail::readTimerfd(timerfd_, system_clock::now());
        if (cb_)
            cb_();
    }

    static struct timespec toTimerSpec(double seconds)
    {
        struct timespec ts;
        memset(&ts, 0, sizeof(ts));
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if (nanoseconds < kMinInterval)
            nanoseconds = kMinInterval;
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    const double interval_;
    TimerCallback cb_;
};


int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid()
             << " Try adjusting the wall clock, see what happens.";
    EventLoop loop;
    PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
    timer.start();
    loop.runEvery(1, std::bind(print, "EventLoop::runEvery"));
    loop.loop();
}