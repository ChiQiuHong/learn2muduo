#include <functional>
#include <memory>

namespace muduo
{

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    namespace net
    {

        // All client visible callbacks go here
        class TcpConnection;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void()> TimerCallback;
        typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
        typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
        typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
        typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;

        typedef std::function<void(const TcpConnectionPtr &)> MessageCallback;

        void defaultConnectionCallback(const TcpConnectionPtr &conn);
        void defaultMessageCallback(const TcpConnectionPtr &conn);

    } // namespace net

} // namespace muduo
