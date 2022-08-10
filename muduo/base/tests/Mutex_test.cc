#include "muduo/base/Mutex.h"

#include <vector>
#include <thread>
#include <stdio.h>

using namespace std;
using namespace muduo;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 1000;

void threadFunc()
{
    for (int i = 0; i < kCount; ++i)
    {
        MutexLockGuard lock(g_mutex);
        g_vec.push_back(i);
    }
}

int main()
{
    thread t1(threadFunc);
    t1.join();
}