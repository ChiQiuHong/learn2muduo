set(net_SRCS
    Address.cc
)

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

install(TARGETS muduo_net DESTINATION lib)

set(HEADERS
    Address.h
)

install(FILES ${HEADERS} DESTINATION include/muduo/net)

add_subdirectory(tests)
# if(MUDUO_BUILD_EXAMPLES)
#     add_subdirectory(tests)
# endif()