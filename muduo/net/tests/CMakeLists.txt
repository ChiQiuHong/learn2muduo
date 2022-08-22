add_executable(acceptor_test Acceptor_test.cc)
target_link_libraries(acceptor_test muduo_net)

if(BOOSTTEST_LIBRARY)
add_executable(address_unittest Address_unittest.cc)
target_link_libraries(address_unittest muduo_net boost_unit_test_framework)
add_test(NAME address_unittest COMMAND address_unittest)
endif()

add_executable(socket_test Socket_test.cc)
target_link_libraries(socket_test muduo_net)

add_executable(tcpserver_test TcpServer_test.cc)
target_link_libraries(tcpserver_test muduo_net)