//
// Created by qiguo on 1/6/18.
//

#ifndef __TINYRPC_NET_H__
#define __TINYRPC_NET_H__

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cross-stdarg.h>

namespace tinynet
{
    // ipv4 address class
    class Ip4Addr
    {
    public:
        Ip4Addr(const std::string & ip, int port) : m_ip (ip), m_port(port), m_valid(true) {}
        Ip4Addr(const sockaddr_in & saddr);
        Ip4Addr() : m_ip(""), m_port(0), m_valid(false) {}
        ~Ip4Addr() = default;

        sockaddr_in pack() const;

        inline std::string ip() const { return m_ip; }
        inline int port() const { return m_port; }
        inline bool valid() const { return m_valid; }

    private:
        std::string     m_ip;
        int             m_port;
        bool            m_valid;    // Is self a valid address ?
    };

    namespace net
    {
        void setNonBlocking(int fd, int nonblocking = true);
    }
}

#endif //TINYRPC_NET_H
