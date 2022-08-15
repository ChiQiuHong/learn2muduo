#pragma once

#include "muduo/base/noncopyable.h"

#include <stdio.h>
#include <iostream>

namespace muduo
{
    const int kSize = 1024*1000;

    class LogFile : noncopyable
    {
    public:
        LogFile(const std::string& basename);
        ~LogFile();

        void append(const char* logline, const size_t len);
        void flush();
    private:
        static std::string getLogFileName(const std::string& basename);

        const std::string basename_;
        char buffer_[kSize];
        FILE* fp_;
    };

} // namespace muduo
