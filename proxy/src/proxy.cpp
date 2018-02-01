//
// Created by qiguo on 1/22/18.
//

#include "proxy.h"

namespace tinyrpc
{
    Proxy::Proxy(const Config &config)
        : m_config(config), m_loop(m_config.main().maxConn())
    {
    }

    void Proxy::reload(const Config &config)
    {
        // TODO
    }

    void Proxy::dispatch(const std::string servername, int fd)
    {
        // TODO
    }

    void Proxy::init(const Config &config)
    {
        // TODO
    }
}
