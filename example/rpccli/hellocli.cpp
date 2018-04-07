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
        msg.data("hhhhhhhhhpapiapiapiapiapiapia");
        debug("req=%s", msg.data().c_str());
        Hello.hello(msg, retval);  // syn call
        debug("resp=%s", retval.data().c_str());

        // asyn call
        Hello.hello(msg, [](Message & retval){
            debug("asyn resp=%s", retval.data().c_str());
        });

        sleep(120);
    }
    catch (std::exception & e)
    {
        info("%s", e.what());
    }
    info("exit");
    return 0;
}
