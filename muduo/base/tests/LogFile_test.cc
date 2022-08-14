#include "muduo/base/LogFile.h"
#include "muduo/base/Logging.h"

#include <unistd.h>

std::unique_ptr<muduo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

void flushFunc()
{
    g_logFile->flush();
}

int main()
{
    const char *file_name = "logFile_test.log";
    g_logFile.reset(new muduo::LogFile(file_name));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);

    std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for (int i = 0; i < 10000; ++i)
    {
        LOG_INFO << line << i;
        
        usleep(1000);
    }
    
}