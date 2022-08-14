#pragma once

#include "muduo/base/noncopyable.h"

#include <stdio.h>

namespace muduo
{
    const int kSize = 1024*1000;

    class LogFile : noncopyable
    {
    public:
        LogFile(const char* filename);
        ~LogFile();

        void append(const char* logline, const size_t len);
        void flush();
    private:
        char buffer_[kSize];
        FILE* fp_;
    };

} // namespace muduo
