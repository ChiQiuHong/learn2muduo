#include "muduo/base/LogStream.h"

#include <sstream>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <boost/timer/timer.hpp>
using namespace boost::timer;
using namespace boost;

const size_t N = 1000000;

#pragma GCC diagnostic ignored "-Wold-style-cast"

template<typename T>
void benchPrintf(const char* fmt)
{   
    auto_cpu_timer t(6, "benchPrintf %w sec\n");
    char buf[32];
    for (size_t i = 0; i < N; ++i)
    {
        snprintf(buf, sizeof(buf), fmt, (T)(i));
    }
}

template<typename T>
void benchStringStream()
{
    auto_cpu_timer t(6, "benchStringStream %w sec\n");
    std::ostringstream os;
    for (size_t i = 0; i < N; ++i)
    {
        os << (T)(i);
        os.seekp(0, std::ios_base::beg);
    }
}

template<typename T>
void benchLogStream()
{
    auto_cpu_timer t(6, "benchLogStream %w sec\n");
    muduo::LogStream os;
    for (size_t i = 0; i < N; ++i)
    {
        os << (T)(i);
        os.resetBuffer();
    }
}

int main()
{
    benchPrintf<int>("%d");

    puts("int");
    benchPrintf<int>("%d");
    benchStringStream<int>();
    benchLogStream<int>();

    puts("double");
    benchPrintf<double>("%.12g");
    benchStringStream<double>();
    benchLogStream<double>();

    puts("int64_t");
    benchPrintf<int64_t>("%" PRId64);
    benchStringStream<int64_t>();
    benchLogStream<int64_t>();

    puts("void*");
    benchPrintf<void*>("%p");
    benchStringStream<void*>();
    benchLogStream<void*>();
}