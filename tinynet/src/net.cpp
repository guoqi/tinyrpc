//
// Created by qiguo on 1/6/18.
//
#include <fcntl.h>
#include "util.h"
#include "net.h"

namespace tinynet
{
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
