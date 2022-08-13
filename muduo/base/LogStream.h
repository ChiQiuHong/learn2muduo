#pragma once

#include "muduo/base/noncopyable.h"

#include <assert.h>
#include <string.h> // memcpy
#include <string>

namespace muduo
{

    namespace detail
    {
        class FixedBuffer : noncopyable
        {
        public:
            FixedBuffer()
                : cur_(data_)
            {
                memset(data_, '\0', sizeof(data_));
            }

            void append(const char *buf, size_t len)
            {
                // FIXME: append partially
                memcpy(cur_, buf, len);
                cur_ += len;
            }

            const char *data() const { return data_; }
            int length() const { return static_cast<int>(cur_ - data_); }

            char *current() { return cur_; }
            void add(size_t len) { cur_ += len; }

            void reset() { cur_ = data_; }

            // for used by unit test
            std::string toString() const { return std::string(data_, length()); }

        private:
            char data_[1024];
            char *cur_;
        };
    } // namespace detail

    class LogStream : noncopyable
    {
    public:
        typedef detail::FixedBuffer Buffer;

        LogStream &operator<<(bool v)
        {
            buffer_.append(v ? "1" : "0", 1);
            return *this;
        }

        LogStream &operator<<(short);              // -2^15~2^15-1 32768~32767
        LogStream &operator<<(unsigned short);     // 0~2^16-1 0~65535
        LogStream &operator<<(int);                // -2^31~2^31-1 -2147483648~2147483647
        LogStream &operator<<(unsigned int);       // 0~2^32-1 0~4294967295
        LogStream &operator<<(long);               // -2^31~2^31-1 -2147483648~2147483647
        LogStream &operator<<(unsigned long);      // 0~2^32-1 0~4294967295
        LogStream &operator<<(long long);          // -2^63~2^63-1 -9223372036854775808~9223372036854775807
        LogStream &operator<<(unsigned long long); // 0~2^64-1 0~18446744073709551615

        LogStream &operator<<(char v)
        {
            buffer_.append(&v, 1);
            return *this;
        }

        LogStream &operator<<(const char *str)
        {
            if (str)
            {
                buffer_.append(str, strlen(str));
            }
            else
            {
                buffer_.append("(null)", 6);
            }
            return *this;
        }

        LogStream &operator<<(const unsigned char *str)
        {
            return operator<<(reinterpret_cast<const char *>(str));
        }

        LogStream &operator<<(const std::string &v)
        {
            buffer_.append(v.c_str(), v.size());
            return *this;
        }

        LogStream &operator<<(const Buffer &v)
        {
            *this << v.toString();
            return *this;
        }

        const Buffer &buffer() const { return buffer_; }
        void resetBuffer() { buffer_.reset(); }

    private:
        template <typename T>
        void formatInteger(T);

        Buffer buffer_;
    };
} // namespace muduo