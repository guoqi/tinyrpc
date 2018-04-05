#include "server.h"
#include <string>

using namespace tinyrpc;


class HelloSvr : public Server
{
public:
    HelloSvr(const std::string & name);

    void initialize();
    void destory();

    void hello(const Message & msg, Message & retval);
};

DECLARE_SERVER_CLASS(HelloSvr);
