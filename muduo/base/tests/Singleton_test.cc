#include "muduo/base/Singleton.h"

#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace muduo;

class Test : noncopyable
{
public:
    Test()
    {
        cout << "tid = " << this_thread::get_id() << " coustructing " << this << endl;
    }

    ~Test()
    {
        cout << "tid = " << this_thread::get_id() << " destructing " << this << " " << name_ << endl;
    }

    void setName(const string &n) { name_ = n; }
    const string &name() const { return name_; }

private:
    string name_;
};

class TestNoDestroy : noncopyable
{
public:
    TestNoDestroy()
    {
        cout << "tid = " << this_thread::get_id() << " coustructing " << this << endl;
    }

    ~TestNoDestroy()
    {
        cout << "tid = " << this_thread::get_id() << " destructing " << this << endl;
    }

    void no_destroy();
};

void threadFunc()
{
    cout << "tid = " << this_thread::get_id() << " name = " << Singleton<Test>::instance().name() << endl;
    Singleton<Test>::instance().setName("only one, changed");
}

int main()
{
    Singleton<Test>::instance().setName("only one");
    cout << "tid = " << this_thread::get_id() << " name = " << Singleton<Test>::instance().name() << endl;
    thread t1(threadFunc);
    t1.join();
    cout << Singleton<Test>::instance().name() << endl;
    Singleton<TestNoDestroy>::instance();
    cout << "With valgrind, you should see " << sizeof(TestNoDestroy) << "-byte memory leak." << endl;
}