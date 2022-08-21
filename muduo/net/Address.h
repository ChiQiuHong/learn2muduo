#pragma once

#include "muduo/base/copyable.h"

#include <string>
#include <netinet/in.h>

namespace muduo
{
    namespace net
    {
        class IPAddress
        {
        public:
            virtual sa_family_t family() const = 0;
            virtual std::string toIp() const = 0;
            virtual std::string toIpPort() const = 0;
            virtual uint16_t port() const = 0;

            // virtual const struct sockaddr *getSockAddr() const = 0;
            // virtual void setSockAddr() = 0;

            virtual uint32_t ipnetEndian() const = 0;
            virtual uint16_t portNetEndian() const = 0;
        };

        class IPv4Address : public IPAddress,
                            public copyable
        {
        public:
            explicit IPv4Address(uint16_t port = 0, bool loopbackOnly = false);

            IPv4Address(std::string ip, uint16_t port);

            explicit IPv4Address(const struct sockaddr_in &addr)
                : addr_(addr)
            {
            }

            sa_family_t family() const { return addr_.sin_family; }
            std::string toIp() const override;
            std::string toIpPort() const override;
            uint16_t port() const override;

            // const struct sockaddr *getSockAddr() const;
            void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

            uint32_t ipnetEndian() const override;
            uint16_t portNetEndian() const override { return addr_.sin_port; }

            static bool resolve(std::string hostname, IPv4Address *result);

        private:
            struct sockaddr_in addr_;
        };

        // class IPv6Address : public IPAddress
        // {
        // public:
        //     explicit IPv6Address(uint16_t port = 0, bool loopback = false);

        //     IPv6Address(std::string ip, uint16_t port);

        //     explicit IPv6Address(const struct sockaddr_in6 &addr)
        //         : addr6_(addr)
        //     {
        //     }

        //     sa_family_t family() const { return addr6_.sin6_family; }
        //     std::string toIp() const override;
        //     std::string toIpPort() const override;
        //     uint16_t port() const override;

        //     const struct sockaddr *getSockAddr() const { return dynamic_cast<sockaddr *>(&addr6_); }
        //     void setSockAddrInet(const struct sockaddr_in6 &addr) { addr6_ = addr; }

        //     uint32_t ipnetEndian() const;
        //     uint16_t portNetEndian() const;

        //     static bool resolve(std::string hostname, IPv6Address *result);

        // private:
        //     sockaddr_in6 addr6_;
        // };

    } // namespace net

} // namespace muduo
