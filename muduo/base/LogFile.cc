#include "muduo/base/LogFile.h"

#include <stdio.h>

namespace muduo
{
    LogFile::LogFile(const char* filename)
        : fp_(::fopen(filename, "a"))
    {
        ::setbuffer(fp_, buffer_, sizeof(buffer_));
    }

    LogFile::~LogFile()
    {
        ::fclose(fp_);
    }

    void LogFile::append(const char* logline, const size_t len)
    {
        ::fwrite_unlocked(logline, 1, len, fp_);
    }

    void LogFile::flush()
    {
        ::fflush(fp_);
    }
} // namespace muduo
