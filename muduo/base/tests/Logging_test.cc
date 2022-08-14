#include "muduo/base/Logging.h"

#include <iostream>

int main()
{
    muduo::Logger::setLogLevel(muduo::Logger::TRACE);
    LOG_TRACE << "This is TRACE level";
    LOG_DEBUG << "This is DEBUG level";
    LOG_INFO << "This is INFO level";
    LOG_WARN << "This is WARN level";
    LOG_ERROR << "This is ERROR level";
    LOG_FATAL << "This is FATAL level";
}