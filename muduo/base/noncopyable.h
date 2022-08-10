#pragma once

// 禁用值语义 不可复制

namespace muduo
{

    class noncopyable
    {
    public:
        noncopyable(const noncopyable &) = delete;
        void operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };

} // namespace muduo