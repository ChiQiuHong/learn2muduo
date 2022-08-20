#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Condition.h"

#include <vector>
#include <stdio.h>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace muduo;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 10 * 1000 * 1000;

void threadFunc()
{
    for (int i = 0; i < kCount; ++i)
    {
        MutexLockGuard lock(g_mutex);
        g_vec.push_back(i);
    }
}

int foo() __attribute__((noinline));

int g_count = 0;
int foo()
{
    MutexLockGuard lock(g_mutex);
    if (!g_mutex.isLockedByThisThread())
    {
        printf("FAIL\n");
        return -1;
    }
    ++g_count;
    return 0;
}

int main()
{
    printf("sizeof pthread_mutex_t: %zd\n", sizeof(pthread_mutex_t));
    printf("sizeof Mutex: %zd\n", sizeof(MutexLock));
    printf("sizeof pthread_cond_t: %zd\n", sizeof(pthread_cond_t));
    printf("sizeof Condition: %zd\n", sizeof(Condition));
    MCHECK(foo());
    if (g_count != 1)
    {
        printf("MCHECK calls twice.\n");
        abort();
    }

    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads * kCount);

    auto start = system_clock::now();
    for (int i = 0; i < kCount; ++i)
    {
        g_vec.push_back(i);
    }
    auto end = system_clock::now();
    auto us = duration_cast<microseconds>(end - start);
    double seconds = static_cast<int>(us.count()) / 1000000.0;
    printf("single thread without lock %fs\n", seconds);

    start = system_clock::now();
    threadFunc();
    end = system_clock::now();
    us = duration_cast<microseconds>(end - start);
    seconds = static_cast<int>(us.count()) / 1000000.0;
    printf("single thread with lock %fs\n", seconds);

    for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads)
    {
        std::vector<std::unique_ptr<Thread>> threads;
        g_vec.clear();
        start = system_clock::now();
        for (int i = 0; i < nthreads; ++i)
        {
            threads.emplace_back(new Thread(&threadFunc));
            threads.back()->start();
        }
        for (int i = 0; i < nthreads; ++i)
        {
            threads[i]->join();
        }
        end = system_clock::now();
        us = duration_cast<microseconds>(end - start);
        seconds = static_cast<int>(us.count()) / 1000000.0;
        printf("%d thread(s) with lock %fs\n", nthreads, seconds);
    }
    
}