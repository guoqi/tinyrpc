//
// Created by qiguo on 1/6/18.
//

#ifndef __TINYRPC_NET_H__
#define __TINYRPC_NET_H__

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cross-stdarg.h>
#include <sys/un.h>

namespace tinynet
{
    class NetAddr
    {
    public:
        NetAddr() = default;
        virtual ~NetAddr() = default;

        virtual sockaddr * addr() const = 0;
        virtual size_t len() const = 0;
    };

    // ipv4 address class
    class Ip4Addr : public NetAddr
    {
    public:
        Ip4Addr(const std::string & ip, int port);
        Ip4Addr(const sockaddr_in & saddr);
        Ip4Addr(): m_ip(""), m_port(0), m_valid(false) {}
        virtual ~Ip4Addr() = default;

        virtual sockaddr * addr() const;
        virtual size_t len() const { return sizeof(sockaddr_in); }

        inline std::string ip() const { return m_ip; }
        inline int port() const { return m_port; }
        inline bool valid() const { return m_valid; }

    private:
        std::string     m_ip;
        int             m_port;
        bool            m_valid;
        sockaddr_in     m_saddr;
    };

    class UdsAddr : public NetAddr
    {
    public:
        explicit UdsAddr(const std::string & sockpath);
        virtual ~UdsAddr() = default;

        virtual sockaddr *addr() const;
        virtual size_t len() const;

        inline std::string path() const { return m_sockpath; }

    private:
        std::string     m_sockpath;
        sockaddr_un     m_saddr;
    };

    namespace net
    {
        void setNonBlocking(int fd, int nonblocking = true);
    }
}

#endif //TINYRPC_NET_H
