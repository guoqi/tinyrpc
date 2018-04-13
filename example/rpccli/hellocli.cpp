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
        msg.dst(1);
        msg.data("11111111111111");
        Hello.hello(msg, retval);  // syn call
        debug("resp=%s", retval.data().c_str());
        msg.data("22222222222222");
        Hello.hello(msg, retval);  // syn call
        debug("resp=%s", retval.data().c_str());

        // asyn call
        msg.data("3333333333333");
        Hello.hello(msg, [](Message & retval){
            debug("asyn resp=%s", retval.data().c_str());
        }, [](const TinyExp & e){
            debug("errhanlder=%s", e.what());
        });

        msg.data("444444444444444");
        Hello.hello(msg, [](Message & retval){
            debug("asyn 2 resp=%s", retval.data().c_str());
        }, [](const TinyExp & e){
            debug("errhandler_2=%s", e.what());
        });

        msg.data("5555555555555555");
        Hello.hello(msg, retval);
        debug("syn resp=%s", retval.data().c_str());

        sleep(10);
    }
    catch (std::exception & e)
    {
        info("%s", e.what());
    }
    info("exit");
    return 0;
}
