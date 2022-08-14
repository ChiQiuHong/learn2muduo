#pragma once

#include "muduo/base/LogStream.h"

#include <memory>

namespace muduo
{
    class Logger
    {
    public:
        enum LogLevel
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        Logger(LogLevel level);
        ~Logger();

        LogStream &stream();

        static LogLevel logLevel();
        static void setLogLevel(LogLevel level);

    private:
        class Impl;
        std::unique_ptr<Impl> pImpl_;
    };

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel()
    {
        return g_logLevel;
    }

#define LOG_TRACE                                          \
    if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
    muduo::Logger(muduo::Logger::TRACE).stream()
#define LOG_DEBUG                                          \
    if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
    muduo::Logger(muduo::Logger::DEBUG).stream()
#define LOG_INFO                                          \
    if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
    muduo::Logger(muduo::Logger::INFO).stream()
#define LOG_WARN muduo::Logger(muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(muduo::Logger::FATAL).stream()

} // namespace muduo
