//
// Created by qiguo on 12/30/17.
//

#ifndef __TINYNET_POLLER_H
#define __TINYNET_POLLER_H

#include "util.h"

namespace tinynet
{
    // Abstract poll action.
    class Poller : public util::noncopyable
    {
    public:
        Poller() {}
        virtual ~Poller() {}
        virtual void addChannel();
    };
}

#endif //TINYRPC_POLLER_H
