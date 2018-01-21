//
// Created by qiguo on 1/6/18.
//
#include <fcntl.h>
#include "util.h"
#include "net.h"

namespace tinynet
{
    Ip4Addr::Ip4Addr(const sockaddr_in &saddr)
    {
        m_ip = inet_ntoa(saddr.sin_addr);
        m_port = ntohs(saddr.sin_port);
        m_valid = true;
    }

    sockaddr_in Ip4Addr::addr() const
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)m_port);
        addr.sin_addr.s_addr = inet_addr(m_ip.c_str());

        return addr;
    }

    sockaddr_un UdsAddr::addr() const
    {
        sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, m_sockpath.c_str(), sizeof(addr.sun_path) - 1);

        return addr;
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
