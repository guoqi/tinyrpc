#include "hello.h"
#include <iostream>

using namespace std;
using namespace std::placeholders;

HelloSvr::HelloSvr()
    : Server()
{
    bind("hello", std::bind(&HelloSvr::hello, this, _1, _2));
}

void HelloSvr::initialize() {}
void HelloSvr::destory() {}

void HelloSvr::hello(const Message &msg, Message &retval)
{
    cout << msg.data() << endl;
    retval.protocol(MESSAGE);
    retval.data("hello world");
}