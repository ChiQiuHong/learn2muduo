#include "muduo/net/Address.h"
#include "muduo/base/Logging.h"

#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

using namespace muduo;
using namespace muduo::net;

IPv4Address::IPv4Address(uint16_t portArg, bool loopbackOnly)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = htobe32(ip);
    addr_.sin_port = htobe16(portArg);
}

IPv4Address::IPv4Address(std::string ip, uint16_t portArg)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htobe16(portArg);
    // 将点分十进制的ip地址转换为用于网络传输的数值格式
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0)
    {
        LOG_ERROR << "IPv4address inet_pton error";
    }
}

std::string IPv4Address::toIp() const
{
    char buf[64] = "";
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string IPv4Address::toIpPort() const
{
    char buf[64] = "";
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = ::strlen(buf);
    uint16_t port = be16toh(addr_.sin_port);
    assert(sizeof(buf) > end);
    snprintf(buf+end, sizeof(buf)-end, ":%u", port);
    return buf;
}

uint16_t IPv4Address::port() const
{
    return be16toh(portNetEndian());
}

// const struct sockaddr *IPv4Address::getSockAddr() const
// {
//     return reinterpret_cast<struct sockaddr*>(static_cast<void*>(&addr_));
// }

uint32_t IPv4Address::ipnetEndian() const
{
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

static __thread char t_resolveBuffer[64 * 1024];

bool IPv4Address::resolve(std::string hostname, IPv4Address *out)
{
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    memset(&hent, 0, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if(ret == 0 && he != NULL) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else {
        if(ret) {
            LOG_ERROR << "InetAddress::resolve";
        }
        return false;
    }
}