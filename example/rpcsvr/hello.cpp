#include "hello.h"
#include <iostream>

using namespace std;

HelloSvr::HelloSvr()
    : Server()
{
    bind("hello", std::bind(HelloSvr::hello, this));
}

void HelloSvr::hello(const Message &msg, Message &retval)
{
    cout << msg.data() << endl;
    retval.protocol(Message);
    retval.data("hello world");
}