#include "muduo/base/ThreadPool.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <thread>

using namespace std;
using namespace muduo;

void print()
{
    cout << "tid: " << this_thread::get_id() << endl;
}

void printString(const std::string& str)
{
    cout << str << endl;
    usleep(100 * 1000);
}

void test(int maxSize)
{
    cout << "Test ThreadPool with max queue size = " << maxSize << endl;
    muduo::ThreadPool pool("mainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    cout << "Adding" << endl;
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "task %d", i);
        pool.run(std::bind(printString, std::string(buf)));
    }
    cout << "Done" << endl;
    // pool.stop();
}

int main()
{
    test(5);
}