add_executable(mutex_test Mutex_test.cc)
target_link_libraries(mutex_test pthread rt)

add_executable(condition_test Condition_test.cc)
target_link_libraries(condition_test pthread rt)