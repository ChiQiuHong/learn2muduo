#include <chrono>
#include <vector>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace chrono;

int main()
{
    auto today = system_clock::now();
    time_t tm = system_clock::to_time_t(today);
    cout << "today is " << ctime(&tm);

    struct tm tm_time;
    localtime_r(&tm, &tm_time);
    char t_time[64];
    // int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
    //                    tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
    //                    tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    // (void)len;

    strftime(t_time, sizeof(t_time), "%Y%m%d %H:%M:%S", &tm_time);
    cout << t_time << endl;

    // seconds
    std::chrono::seconds seconds = duration_cast<std::chrono::seconds>(today.time_since_epoch());
    cout << seconds.count() << endl;

    // microseconds
    microseconds cs;
    cs = duration_cast<std::chrono::microseconds>(today.time_since_epoch()) % 1000000;
    cout << cs.count() << endl;

    auto now = system_clock::now();
    time_t time = system_clock::to_time_t(now);
    cout << "Now is: " << ctime(&time) << endl;

    auto start = steady_clock::now();
    // do something
    auto end = steady_clock::now();

    auto time_diff = end - start;
    auto duration = duration_cast<std::chrono::seconds>(time_diff);
    cout << "Operation cost : " << duration.count() << "s" << endl;
}