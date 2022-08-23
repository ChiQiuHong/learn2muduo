set(net_SRCS
    Acceptor.cc
    Address.cc
    Channel.cc
    EPoller.cc
    Socket.cc
    TcpConnection.cc
    TcpServer.cc
)

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

install(TARGETS muduo_net DESTINATION lib)

set(HEADERS
    Address.h
    Callbacks.h
    Channel.h
    TcpConnection.h
    TcpServer.h
)

install(FILES ${HEADERS} DESTINATION include/muduo/net)

if(MUDUO_BUILD_EXAMPLES)
    add_subdirectory(tests)
endif()