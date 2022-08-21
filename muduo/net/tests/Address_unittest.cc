#include "muduo/net/Address.h"
#include "muduo/base/Logging.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>

using namespace muduo::net;

BOOST_AUTO_TEST_CASE(testInetAddress)
{
    IPv4Address addr0(1234);
    BOOST_CHECK_EQUAL(addr0.toIp(), std::string("0.0.0.0"));
    BOOST_CHECK_EQUAL(addr0.toIpPort(), std::string("0.0.0.0:1234"));
    BOOST_CHECK_EQUAL(addr0.port(), 1234);
}