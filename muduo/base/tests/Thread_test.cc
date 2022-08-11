#include "muduo/base/Thread.h"

#include <string>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

using namespace std;
using namespace muduo;

void threadFunc()
{
    cout << "tid=" << this_thread::get_id() << endl;
}

void threadFunc2(int x)
{
    cout << "tid=" << this_thread::get_id() << ", x=" << x << endl;
}

class Foo
{
public:
    explicit Foo(double x)
        : x_(x)
    {
    }
    
    void memberFunc()
    {
        cout << "tid=" << this_thread::get_id() << ", Foo::x=" << x_ << endl;
    }

private:
    double x_;
};

int main()
{
    cout << "pid=" << getpid() << ", "
         << "std::thread tid=" << this_thread::get_id() << ", "
         << "syscall tid=" << syscall(SYS_gettid) << endl;

    muduo::Thread t1(threadFunc);
    t1.start();
    cout << "t1.tid=" << t1.tid() << endl;
    t1.join();

    muduo::Thread t2(std::bind(threadFunc2, 42),
                     "thread for free function with argument");
    t2.start();
    cout << "t1.tid=" << t1.tid() << endl;
    cout << "t2.tid=" << t2.tid() << endl;
    t2.join();

    Foo foo(87.53);
    muduo::Thread t3(std::bind(&Foo::memberFunc, &foo),
                     "thread for member function without argument");
    t3.start();
    t3.join();
}