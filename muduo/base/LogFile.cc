#include "muduo/base/LogFile.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <chrono>

using namespace std::chrono;

namespace muduo
{
    LogFile::LogFile(const std::string &basename,
                     off_t rollSize,
                     int flushInterval,
                     int checkEveryN)
        : basename_(basename),
          rollSize_(rollSize),
          writtenBytes_(0),
          flushInterval_(flushInterval),
          checkEveryN_(checkEveryN),
          count_(0),
          startOfPeriod_(0),
          lastRoll_(0),
          lastFlush_(0)
    {
        rollFile();
    }

    LogFile::~LogFile()
    {
        ::fclose(fp_);
    }

    void LogFile::append(const char *logline, const size_t len)
    {
        size_t written = 0;
        while (written != len)
        {
            size_t remain = len - written;
            size_t n = ::fwrite_unlocked(logline, 1, len, fp_);
            if (n != remain)
            {
                int err = ferror(fp_);
                if (err)
                {
                    fprintf(stderr, "LogFile::append() failed %s\n", strerror(err));
                    break;
                }
            }
            written += n;
        }
        
        writtenBytes_ += written;

        if (writtenBytes_ > rollSize_)
        {
            writtenBytes_ = 0;
            rollFile();
        }
        else
        {
            ++count_;
            if (count_ >= checkEveryN_)
            {
                count_ = 0;
                auto now = system_clock::now();
                time_t now_t = system_clock::to_time_t(now);
                time_t thisPeriod_ = now_t / kRollPerSeconds_ * kRollPerSeconds_;
                if (thisPeriod_ != startOfPeriod_)
                {
                    rollFile();
                }
                else if (now_t - lastFlush_ > flushInterval_)
                {
                    lastFlush_ = now_t;
                    flush();
                }
            }
        }
    }

    void LogFile::flush()
    {
        ::fflush(fp_);
    }

    bool LogFile::rollFile()
    {
        time_t now_t = 0;
        std::string filename = getLogFileName(basename_, &now_t);
        // 将start对齐到kR的整数倍，也就是时间调整到当天零时
        // now是1970.1.1零时到现在的秒数
        time_t start = now_t / kRollPerSeconds_ * kRollPerSeconds_;

        if (now_t > lastRoll_)
        {
            lastRoll_ = now_t;
            lastFlush_ = now_t;
            startOfPeriod_ = start;
            if (fp_)
            {
                ::fclose(fp_);
            }
            fp_ = ::fopen(filename.c_str(), "a");
            memset(buffer_, 0, sizeof(buffer_));
            ::setbuffer(fp_, buffer_, sizeof(buffer_));
            return true;
        }
        return false;
    }

    std::string LogFile::getLogFileName(const std::string &basename, time_t *now_t)
    {
        std::string filename;
        filename.reserve(basename.size() + 64);
        filename = basename;

        char timebuf[32];
        struct tm tm;
        auto now = system_clock::now();
        *now_t = system_clock::to_time_t(now);
        localtime_r(now_t, &tm);
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
