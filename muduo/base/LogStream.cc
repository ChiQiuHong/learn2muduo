#include "muduo/base/LogStream.h"

#include <stdio.h>
#include <algorithm>
#include <stdint.h> // uintptr_t

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

namespace muduo
{

    namespace detail
    {
        const char digits[] = "9876543210123456789";
        const char *zero = digits + 9; // zero 指向 '0'
        static_assert(sizeof(digits) == 20, "wrong number of digits");

        const char digitsHex[] = "0123456789ABCDEF";
        static_assert(sizeof(digitsHex) == 17, "wrong number of digitsHex");

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

        size_t converHex(char buf[], uintptr_t value)
        {
            uintptr_t i = value;
            char *p = buf;

            do
            {
                int lsd = static_cast<int>(i % 16);
                i /= 16;
                *p++ = digitsHex[lsd];
            } while (i != 0);

            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }

        template class FixedBuffer<kSmallBuffer>;
        template class FixedBuffer<kLargeBuffer>;

    } // namespace detail

    template <typename T>
    void LogStream::formatInteger(T v)
    {
        size_t len = detail::convert(buffer_.current(), v);
        buffer_.add(len);
    }

    LogStream &LogStream::operator<<(short v)
    {
        *this << static_cast<int>(v);
        return *this;
    }

    LogStream &LogStream::operator<<(unsigned short v)
    {
        *this << static_cast<unsigned int>(v);
        return *this;
    }

    LogStream &LogStream::operator<<(int v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(unsigned int v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(unsigned long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(long long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(unsigned long long v)
    {
        formatInteger(v);
        return *this;
    }

    LogStream &LogStream::operator<<(const void *p)
    {
        uintptr_t v = reinterpret_cast<uintptr_t>(p);
        if (buffer_.avail() >= kMaxNumericSize)
        {
            char *buf = buffer_.current();
            buf[0] = '0';
            buf[1] = 'x';
            size_t len = detail::converHex(buf + 2, v);
            buffer_.add(len + 2);
        }
        return *this;
    }

    LogStream &LogStream::operator<<(double v)
    {
        if (buffer_.avail() >= kMaxNumericSize)
        {
            int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
            buffer_.add(len);
        }
        return *this;
    }

} // namespace muduo
