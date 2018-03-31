#include "server.h"

using namespace tinyrpc;


class HelloSvr : public Server
{
public:
    HelloSvr();

    void hello(const Message & msg, Message & retval);
};

DECLARE_SERVER_CLASS(HelloSvr);
