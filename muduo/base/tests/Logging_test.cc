#include "muduo/base/Logging.h"
#include "muduo/base/LogFile.h"

#include <iostream>
#include <chrono>
#include <stdio.h>
#include <unistd.h>

using namespace std::chrono;

int g_total;
FILE *g_file;
std::unique_ptr<muduo::LogFile> g_logFile;

void dummyOutput(const char *msg, int len)
{
    g_total += len;
    if (g_file)
    {
        fwrite(msg, 1, len, g_file);
    }
    else if (g_logFile)
    {
        g_logFile->append(msg, len);
    }
}

void bench(const char *type)
{
    muduo::Logger::setOutput(dummyOutput);
    auto start = system_clock::now();
    g_total = 0;

    int n = 1000 * 1000;
    const bool kLongLog = false;
    std::string empty = " ";
    std::string longStr(3000, 'X');
    longStr += " ";
    for (int i = 0; i < n; ++i)
    {
        LOG_INFO << "Hello 0123456789"
                 << " abcdefghijklmnopqrstuvwxyz"
                 << (kLongLog ? longStr : empty)
                 << i;
    }
    auto end = system_clock::now();
    auto time_diff = end - start;
    auto us = duration_cast<microseconds>(time_diff);
    double seconds = static_cast<int>(us.count()) / 1000000.0;
    // std::cout << type << seconds << " seconds, " << g_total << " bytes, "
    //           << n / seconds << " msg/s, "
    //           << g_total / seconds / (1024 * 1024) << " MiB/s" << std::endl;
    printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
           type, seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

int main()
{
    muduo::Logger::setLogLevel(muduo::Logger::TRACE);
    LOG_TRACE << "This is TRACE level";
    LOG_DEBUG << "This is DEBUG level";
    LOG_INFO << "This is INFO level";
    LOG_WARN << "This is WARN level";
    LOG_ERROR << "This is ERROR level";
    LOG_FATAL << "This is FATAL level";

    sleep(1);
    bench("nop");

    char buffer[64 * 1024];

    g_file = fopen("/dev/null", "w");
    setbuffer(g_file, buffer, sizeof(buffer));
    bench("/dev/null");
    fclose(g_file);

    g_file = fopen("/tmp/log", "w");
    setbuffer(g_file, buffer, sizeof(buffer));
    bench("/tmp/log");
    fclose(g_file);
}