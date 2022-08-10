# set(base_SRCS
#     Mutex.h
# )

# add_library(muduo_base ${base_SRCS})
# target_link_libraries(muduo_base pthread rt)

# # install(TARGET muduo_base DESTINATION lib)

# file(GLOB HEADERS "*.h")
# install(FILES ${HEADERS} DESTINATION include/muduo/base)

add_subdirectory(tests)