//
// Created by qiguo on 1/6/18.
//
#include <fcntl.h>
#include "util.h"
#include "net.h"

namespace tinynet
{
    Ip4Addr::Ip4Addr(const std::string &ip, int port)
        : m_ip (ip), m_port(port), m_valid(true)
    {
        memset(&m_saddr, 0, sizeof(addr));
        m_saddr.sin_family = AF_INET;
        m_saddr.sin_port = htons((uint16_t)m_port);
        m_saddr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    }

    Ip4Addr::Ip4Addr(const sockaddr_in &saddr)
        : m_saddr(saddr)
    {
        m_ip = inet_ntoa(saddr.sin_addr);
        m_port = ntohs(saddr.sin_port);
        m_valid = true;
    }

    virtual sockaddr * Ip4Addr::addr() const
    {
        return (sockaddr *)&m_saddr;
    }

    UdsAddr::UdsAddr(const std::string &sockpath)
        : m_sockpath(sockpath)
    {
        memset(&m_saddr, 0, sizeof(addr));
        m_saddr.sun_family = AF_UNIX;
        strncpy(m_saddr.sun_path, m_sockpath.c_str(), sizeof(addr.sun_path) - 1);
    }

    virtual sockaddr * UdsAddr::addr() const
    {
        return (sockaddr *)&m_saddr;
    }

    size_t UdsAddr::len() const
    {
        return offsetof(struct sockaddr_un, sun_path) + m_sockpath.length();
    }

    namespace net
    {
        void setNonBlocking(int fd, int nonblocking)
        {
            int flags = fcntl(fd, F_GETFL, 0);
            if (nonblocking)
            {
                flags |= O_NONBLOCK;
            }
            else
            {
                flags &= ~O_NONBLOCK;
            }

            fatalif(fcntl(fd, F_SETFL, flags) == -1);
        }
    }
}
