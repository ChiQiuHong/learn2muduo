set(net_SRCS
    Acceptor.cc
    Address.cc
    Buffer.cc
    Channel.cc
    EPoller.cc
    EventLoop.cc
    EventLoopThread.cc
    EventLoopThreadPool.cc
    Socket.cc
    TcpConnection.cc
    TcpServer.cc
    Timer.cc
    TimerQueue.cc
)

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

install(TARGETS muduo_net DESTINATION lib)

set(HEADERS
    Address.h
    Buffer.h
    Callbacks.h
    Channel.h
    EventLoop.h
    EventLoopThread.cc
    EventLoopThreadPool.h
    TcpConnection.h
    TcpServer.h
    TimerId.h
)

install(FILES ${HEADERS} DESTINATION include/muduo/net)

if(MUDUO_BUILD_EXAMPLES)
    add_subdirectory(tests)
endif()