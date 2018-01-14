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

    sockaddr_in Ip4Addr::pack() const
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)m_port);
        addr.sin_addr.s_addr = inet_addr(m_ip.c_str());

        return addr;
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
