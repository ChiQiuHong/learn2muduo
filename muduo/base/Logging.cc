#include "muduo/base/Logging.h"

#include <iostream>
#include <chrono>

using namespace std::chrono;

namespace muduo
{
    char t_time[64];
    std::chrono::seconds t_lastSecond;

    const char *LogLevelName[Logger::NUM_LOG_LEVELS]{
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
    };

    class Logger::Impl
    {
    public:
        typedef Logger::LogLevel LogLevel;

        Impl(LogLevel level, const char* name, int line)
            : time_(system_clock::now()),
              stream_(),
              level_(level),
              line_(line),
              basename_(name)
        {
            // FIXME tid
            formatTime();
            formatFileName();
            stream_ << LogLevelName[level];
        }

        void formatTime()
        {
            auto seconds = duration_cast<std::chrono::seconds>(time_.time_since_epoch());
            if (seconds != t_lastSecond)
            {
                t_lastSecond = seconds;
                time_t time = system_clock::to_time_t(time_);
                struct tm tm_time;
                ::localtime_r(&time, &tm_time);

                strftime(t_time, sizeof(t_time), "%Y%m%d %H:%M:%S", &tm_time);
            }

            // microseconds
            auto us = duration_cast<microseconds>(time_.time_since_epoch()) % 1000000;
            char buf[20];
            snprintf(buf, sizeof(buf), ".%06ldZ ", us.count());
            stream_ << t_time << buf;
        }

        void formatFileName()
        {
            const char *slash = strrchr(basename_, '/'); // builtin function
            if (slash)
            {
                basename_ = slash + 1;
            }
        }

        void finish()
        {
            stream_ << " - " << basename_ << ":" << line_ << '\n';
        }

        LogStream &stream() { return stream_; }

    private:
        time_point<system_clock> time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        const char* basename_;
    };

    void defaultOutput(const char* msg, int len)
    {
        size_t n = ::fwrite(msg, 1, len, stdout);
        // FIXME check n
        (void)n;
    }

    void defaultFlush()
    {
        ::fflush(stdout);
    }

    Logger::OutputFunc g_output = defaultOutput;
    Logger::FlushFunc g_flush = defaultFlush;

    Logger::Logger(const char* name, int line, LogLevel level)
    {
        pImpl_.reset(new Impl(level, name, line));
    }

    Logger::~Logger()
    {
        pImpl_->finish();
        const LogStream::Buffer &buf(stream().buffer());
        g_output(buf.data(), buf.length());
    }

    LogStream &Logger::stream()
    {
        return pImpl_->stream();
    }

    Logger::LogLevel initLogLevel()
    {
        return Logger::INFO;
    }

    Logger::LogLevel g_logLevel = initLogLevel();

    void Logger::setLogLevel(Logger::LogLevel level)
    {
        g_logLevel = level;
    }

    void Logger::setOutput(OutputFunc out)
    {
        g_output = out;
    }

    void Logger::setFlush(FlushFunc flush)
    {
        g_flush = flush;
    }

} // namespace muduo