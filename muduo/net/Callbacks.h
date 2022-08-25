#include <functional>
#include <memory>

namespace muduo
{

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    template <typename T>
    inline T *get_pointer(const std::shared_ptr<T> &ptr)
    {
        return ptr.get();
    }

    template <typename T>
    inline T *get_pointer(const std::unique_ptr<T> &ptr)
    {
        return ptr.get();
    }

    namespace net
    {

        // All client visible callbacks go here
        class Buffer;
        class TcpConnection;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void()> TimerCallback;
        typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
        typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
        typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
        typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;

        // the data has been read to (buf, len)
        typedef std::function<void(const TcpConnectionPtr &,
                                   Buffer *,
                                   system_clock::time_point)>
            MessageCallback;

        void defaultConnectionCallback(const TcpConnectionPtr &conn);
        void defaultMessageCallback(const TcpConnectionPtr &conn,
                                    Buffer* buffer,
                                    system_clock::time_point receiveTime);

    } // namespace net

} // namespace muduo
