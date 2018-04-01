#include "server.h"

using namespace tinyrpc;


class HelloSvr : public Server
{
public:
    HelloSvr();

    void initialize();
    void destory();

    void hello(const Message & msg, Message & retval);
};

DECLARE_SERVER_CLASS(HelloSvr);
