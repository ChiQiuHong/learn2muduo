#include <functional>
#include <memory>

namespace muduo
{
    namespace net
    {
        
        // All client visible callbacks go here
        class TcpConnection;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void(const TcpConnectionPtr &)> MessageCallback;
    } // namespace net
    
} // namespace muduo
