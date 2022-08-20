#include "muduo/base/CurrentThread.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

namespace
{
__thread int x = 0;
}

void print()
{
    printf("pid=%d tid=%d x=%d\n", getpid(), muduo::CurrentThread::tid(), x);
}

int main()
{
    printf("parent %d\n", getpid());
    print();
    x = 1;
    print();
    pid_t p = fork();   // 在原来的进程中返回新进程的pid，在新进程中返回0

    if (p == 0)
    {
        printf("child %d\n", getpid());
        // child
        print();
        x = 2;
        print();

        if (fork() == 0)
        {
            printf("grandchild %d\n", getpid());
            print();
            x = 3;
            print();
        }
    }
    else
    {
        // parent
        print();
    }
}