#include "muduo/base/Thread.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Atomic.h"

#include <assert.h>
#include <atomic>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;
using namespace muduo;

atomic<int> std_i;
muduo::AtomicInt32 i;
const int maxCnt = 1000000;
muduo::MutexLock g_mutex;

void StdAtomicFunc()
{
    for (int j = 0; j < maxCnt; j++)
    {
        std_i++;
    }
}

void AtomicFunc()
{
    for (int j = 0; j < maxCnt; j++)
    {
        i.increment();
    }
}

void benchStdAtomic()
{
    auto start = system_clock::now();
    muduo::Thread t1(StdAtomicFunc);
    t1.start();
    t1.join();
    muduo::Thread t2(StdAtomicFunc);
    t2.start();
    t2.join();
    auto end = system_clock::now();
    auto us = duration_cast<microseconds>(end - start);
    double seconds = static_cast<int>(us.count()) / 1000000.0;
    cout << "i = " << std_i << endl;
    cout << "time: " << seconds << "s" << endl;
}

void benchAtomic()
{
    auto start = system_clock::now();
    muduo::Thread t1(AtomicFunc);
    t1.start();
    t1.join();
    muduo::Thread t2(AtomicFunc);
    t2.start();
    t2.join();
    auto end = system_clock::now();
    auto us = duration_cast<microseconds>(end - start);
    double seconds = static_cast<int>(us.count()) / 1000000.0;
    cout << "i = " << i.get() << endl;
    cout << "time: " << seconds << "s" << endl;
}

int main()
{
    {
        muduo::AtomicInt64 a0;
        assert(a0.get() == 0);
        assert(a0.getAndAdd(1) == 0);
        assert(a0.get() == 1);
        assert(a0.addAndGet(2) == 3);
        assert(a0.get() == 3);
        assert(a0.incrementAndGet() == 4);
        assert(a0.get() == 4);
        a0.increment();
        assert(a0.get() == 5);
        assert(a0.addAndGet(-3) == 2);
        assert(a0.getAndSet(100) == 2);
        assert(a0.get() == 100);
    }

    {
        muduo::AtomicInt32 a1;
        assert(a1.get() == 0);
        assert(a1.getAndAdd(1) == 0);
        assert(a1.get() == 1);
        assert(a1.addAndGet(2) == 3);
        assert(a1.get() == 3);
        assert(a1.incrementAndGet() == 4);
        assert(a1.get() == 4);
        a1.increment();
        assert(a1.get() == 5);
        assert(a1.addAndGet(-3) == 2);
        assert(a1.getAndSet(100) == 2);
        assert(a1.get() == 100);
    }

    puts("std::atomic");
    benchStdAtomic();

    puts("gcc atomic");
    benchAtomic();
}