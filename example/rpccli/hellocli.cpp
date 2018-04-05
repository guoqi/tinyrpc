#include "client.h"
#include <iostream>

using namespace std;
using namespace tinyrpc;
using namespace tinynet;
using namespace util;

// demo
struct HelloSvr : public Client
{
    HelloSvr(): Client("HelloSvr"),
                INIT_MEM_FUNC(hello)
    {}

    DECLARE_MEM_FUNC(hello);
};


int main()
{
    try
    {
        HelloSvr Hello;
        Message msg, retval;
        msg.data("piapiapiapiapia");
        Hello.hello(msg, retval);  // syn call
    }
    catch (std::exception & e)
    {
        info("%s", e.what());
    }
    return 0;
}
