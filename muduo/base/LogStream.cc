#include "muduo/base/LogStream.h"

#include <stdio.h>
#include <algorithm>

namespace muduo
{

    namespace detail
    {
        const char digits[] = "9876543210123456789";
        const char* zero = digits + 9; // zero 指向 '0'

        // Efficient Integer to String Conversions, by Mathew Wilson.
        template <typename T>
        size_t convert(char buf[], T value)
        {
            T i = value;
            char *p = buf;

            do
            {
                int lsd = static_cast<int>(i % 10); // lsd 可能小于0
                i /= 10;                            // 是向零取整
                *p++ = zero[lsd];                   // 下标可能为负
            } while (i != 0);

            if (value < 0)
            {
                *p++ = '-';
            }
            *p = '\0';
            std::reverse(buf, p);
            return p - buf; // 返回整数长度
        }

    } // namespace detail

    template <typename T>
    void LogStream::formatInteger(T v)
    {
        size_t len = detail::convert(buffer_.current(), v);
        buffer_.add(len);
    }

    LogStream& LogStream::operator<<(short v)
    {
        *this << static_cast<int>(v);
        return *this;
    }

    LogStream& LogStream::operator<<(unsigned short v)
    {
        *this << static_cast<unsigned int>(v);
        return *this;
    }

    LogStream &LogStream::operator<<(int v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream& LogStream::operator<<(unsigned int v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream& LogStream::operator<<(unsigned long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(long long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream& LogStream::operator<<(unsigned long long v)
    {
        formatInteger(v);
        return *this;
    }

} // namespace muduo
