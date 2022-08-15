#include "muduo/base/LogFile.h"

#include <stdio.h>
#include <unistd.h>
#include <chrono>

using namespace std::chrono;

namespace muduo
{
    LogFile::LogFile(const std::string& basename)
        : basename_(basename)
    {
        std::string filename = getLogFileName(basename);
        fp_ = ::fopen(filename.c_str(), "a");
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

    std::string LogFile::getLogFileName(const std::string& basename)
    {
        std::string filename;
        filename.reserve(basename.size() + 64);
        filename = basename;

        char timebuf[32];
        struct tm tm;
        auto now = system_clock::now();
        time_t now_t = system_clock::to_time_t(now);
        localtime_r(&now_t, &tm);
        strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
        filename += timebuf;

        char buf[256];
        if (::gethostname(buf, sizeof(buf)) == 0)
        {
            buf[sizeof(buf) - 1] = '\0';
        }

        filename += buf;
        filename += ".log";

        return filename;
    }
} // namespace muduo
