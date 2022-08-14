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

    Logger::Logger(LogLevel level)
    {
        pImpl_.reset(new Impl(level));
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