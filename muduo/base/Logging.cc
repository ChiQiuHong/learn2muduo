#include "muduo/base/Logging.h"

#include <iostream>

namespace muduo
{

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

        Impl(LogLevel level)
            : stream_(),
              level_(level),
              line_(__LINE__)
        {
            // FIXME time & tid
            stream_ << LogLevelName[level];
        }

        const char *formatFileName()
        {
            const char *filename = __FILE__;
            const char *slash = strrchr(filename, '/'); // builtin function
            if (slash)
            {
                filename = slash + 1;
            }
            return filename;
        }

        void finish()
        {
            stream_ << " - " << formatFileName() << ":" << line_ << '\n';
        }

        LogStream &stream() { return stream_; }

    private:
        LogStream stream_;
        LogLevel level_;
        int line_;
    };

    Logger::Logger(LogLevel level)
    {
        pImpl_.reset(new Impl(level));
    }

    Logger::~Logger()
    {
        pImpl_->finish();
        const LogStream::Buffer &buf(stream().buffer());
        std::cout << buf.data();
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

} // namespace muduo