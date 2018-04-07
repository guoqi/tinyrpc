//
// Created by qiguo on 4/2/18.
//

#ifndef __TINYRPC_CLIENT_H__
#define __TINYRPC_CLIENT_H__

#include "rpcconn.h"
#include "util.h"

using namespace std;

#define DECLARE_MEM_FUNC(name) const SvcFunctor name;
#define INIT_MEM_FUNC(name) name(this, #name)

namespace tinyrpc
{

    class Client : public util::noncopyable
    {
    public:

        class SvcFunctor
        {
        public:
            explicit SvcFunctor(const Client * belongto, const string & service_name): m_server(belongto->name()), m_service(service_name) {}
            virtual ~SvcFunctor() = default;

            // syn rpc call operator
            void operator() (const Message & msg, Message & retval) const
            {
                Connector::instance().call(m_server + "." + m_service, msg, retval);
            }

            // asyn rpc call opeartor
            void operator() (const Message & msg, const RecvCallback & cb, const ErrorCallback & errhandler) const
            {
                Connector::instance().asyn_call(m_server + "." + m_service, msg, cb, errhandler);
            }

        private:
            string      m_server;
            string      m_service;
        };

    public:
        Client(const std::string & server): m_server(server) {}
        virtual ~Client() = default;

        virtual string name() const { return m_server; }

    private:
        std::string             m_server;
    };

}

#endif //TINYRPC_CLIENT_H
