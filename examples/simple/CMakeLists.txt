add_executable(simple_discard discard/discard.cc discard/main.cc)
target_link_libraries(simple_discard muduo_net)
