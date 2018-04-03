//
// Created by qiguo on 4/2/18.
//

#ifndef __TINYRPC_CLIENT_H__
#define __TINYRPC_CLIENT_H__

#include "rpcconn.h"
#include "util.h"

namespace tinyrpc
{

    class Client : public util::noncopyable
    {
    public:
        Client() = default;
        virtual ~Client() = default;

    protected:

    private:
        std::string             m_server;
    };

}

#endif //TINYRPC_CLIENT_H
