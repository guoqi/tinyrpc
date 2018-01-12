//
// Created by qiguo on 12/30/17.
//

#ifndef __TINYNET_CHANNEL_H
#define __TINYNET_CHANNEL_H

#include <functional>
#include "util.h"
#include "event.h"

namespace tinynet
{

    using EventCallback  = std::function<void()>;

    // A channel is a socket-like object which has read and write callback
    class Channel : util::noncopyable
    {
    public:
        Channel(int fd);
        Channel(int fd, EventCallback readcb, EventCallback writecb);
        virtual ~Channel() {}

    private:
        int             m_fd;
        EventCallback   m_readcb;
        EventCallback   m_wiretcb;
    };
}

#endif //TINYRPC_CHANNEL_H
