#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Logging.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

using namespace muduo;
using namespace muduo::net;

int cnt = 0;
EventLoop* g_loop;
char buf[128];

char* formatTime()
{
    memset(buf, 0, sizeof(buf));
    auto now = system_clock::now();
    time_t time = system_clock::to_time_t(now);
    struct tm tm_time;
    ::localtime_r(&time, &tm_time);

    size_t len = strftime(buf, sizeof(buf), "%Y%m%d-%H:%M:%S", &tm_time);
    auto us = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;
    snprintf(buf+len, sizeof(buf), ".%06ldZ ", us.count());

    return buf;
}

void printTid()
{
    printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    printf("now %s\n", formatTime());
}

void print(const char* msg)
{
    printf("msg %s %s\n", formatTime(), msg);
    if (++cnt == 20)
    {
        g_loop->quit();
    }
}

void cancel(TimerId timer)
{
    g_loop->cancel(timer);
    printf("cancelled at %s\n", formatTime());
}

int main()
{
    printTid();
    sleep(1);
    {
        EventLoop loop;
        g_loop = &loop;

        print("main");
        loop.runAfter(1, std::bind(print, "oncel"));
        loop.runAfter(1.5, std::bind(print, "once1.5"));
        loop.runAfter(2.5, std::bind(print, "once2.5"));
        loop.runAfter(3.5, std::bind(print, "once3.5"));
        TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
        loop.runAfter(4.2, std::bind(cancel, t45));
        loop.runAfter(4.8, std::bind(cancel, t45));
        loop.runEvery(2, std::bind(print, "every2"));
        TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
        loop.runAfter(9.001, std::bind(cancel, t3));

        loop.loop();
        print("main loop exits");
    }
    sleep(1);
    {
        EventLoopThread loopThread;
        EventLoop* loop = loopThread.startLoop();
        loop->runAfter(2, printTid);
        sleep(3);
        print("thread loop exits");
    }
}