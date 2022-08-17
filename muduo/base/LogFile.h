#pragma once

#include "muduo/base/noncopyable.h"

#include <stdio.h>
#include <iostream>

namespace muduo
{
    const int kSize = 64 * 1024;

    class LogFile : noncopyable
    {
    public:
        LogFile(const std::string &basename,
                off_t rollSize,
                int flushInterval = 3,
                int checkEveryN = 1024);
        ~LogFile();

        void append(const char *logline, const size_t len);
        void flush();
        bool rollFile();

    private:
        static std::string getLogFileName(const std::string &basename, time_t *now);

        const std::string basename_;
        const off_t rollSize_;
        off_t writtenBytes_;
        const int flushInterval_;
        const int checkEveryN_;

        int count_;

        time_t startOfPeriod_;
        time_t lastRoll_;
        time_t lastFlush_;

        char buffer_[kSize];
        FILE *fp_;

        const static int kRollPerSeconds_ = 60 * 60 * 24;
    };

} // namespace muduo
