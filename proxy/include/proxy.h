//
// Created by qiguo on 1/21/18.
//

#ifndef __TINYRPC_PROXY_H__
#define __TINYRPC_PROXY_H__

#include "util.h"
#include "config.h"
#include "event.h"

namespace tinyrpc
{
    class Proxy : util::noncopyable
    {
    public:
        Proxy(const Config & config);

        void start() { m_loop.start(); }
        void stop() { m_loop.stop(); }

        // reload configuration
        void reload(const Config & config);

        // dispatch request to a specific server process
        void dispatch(const std::string servername, int fd);

    protected:
        void handleRead(tinynet::EventLoop & loop, tinynet::Event & ev);

        void init(const Config & config);

    private:
        const Config            m_config;
        tinynet::EventLoop      m_loop;
    };
}

#endif //TINYRPC_RPCPROXY_H
