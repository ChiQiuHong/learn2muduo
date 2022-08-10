#include "muduo/base/Condition.h"

#include <iostream>
#include <thread>
#include <queue>

using namespace std;
using namespace muduo;

MutexLock g_mutex;              // 全局互斥锁
queue<int> g_que;               // 全局消息队列
Condition g_condition(g_mutex); // 全局条件变量
int cnt = 1;                    // 数据

void producer()
{
    while (true)
    {
        {
            MutexLockGuard lock(g_mutex);
            g_que.push(cnt);
            cout << "向队列中添加数据：" << cnt++ << endl;
        } // 用大括号括起来，lock会自动解锁
        g_condition.notifyAll();
    }
}

void consumer()
{
    while (true)
    {
        MutexLockGuard lock(g_mutex);
        while (g_que.size() == 0)   // 防止虚假唤醒，这里再判断一次
        {
            g_condition.wait();
        }
        int tmp = g_que.front();
        cout << "从队列中取出数据：" << tmp << endl;
        g_que.pop();
    }
}

int main()
{
    thread thd1[2], thd2[2];
    for (int i = 0; i < 2; i++)
    {
        thd1[i] = thread(producer);
        thd2[i] = thread(consumer);
        thd1[i].join();
        thd2[i].join();
    }

    return 0;
}