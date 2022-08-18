add_executable(asynclogging_test AsyncLogging_test.cc)
target_link_libraries(asynclogging_test muduo_base)

add_executable(condition_test Condition_test.cc)
target_link_libraries(condition_test muduo_base)

add_executable(logfile_test LogFile_test.cc)
target_link_libraries(logfile_test muduo_base)

add_executable(logging_test Logging_test.cc)
target_link_libraries(logging_test muduo_base)

add_executable(logstream_bench LogStream_bench.cc)
target_link_libraries(logstream_bench muduo_base boost_timer)

if(BOOSTTEST_LIBRARY)
add_executable(logstream_test LogStream_test.cc)
target_link_libraries(logstream_test muduo_base boost_unit_test_framework)
add_test(NAME logstream_test COMMAND logstream_test)
endif()

add_executable(mutex_test Mutex_test.cc)
target_link_libraries(mutex_test muduo_base)

add_executable(singleton_test Singleton_test.cc)
target_link_libraries(singleton_test muduo_base)

add_executable(thread_test Thread_test.cc)
target_link_libraries(thread_test muduo_base)

add_executable(threadpool_test ThreadPool_test.cc)
target_link_libraries(threadpool_test muduo_base)

add_executable(timestamp_unittest TimeStamp_unittest.cc)
target_link_libraries(timestamp_unittest muduo_base)
# add_test(NAME timestamp_unittest COMMAND timestamp_unittest)