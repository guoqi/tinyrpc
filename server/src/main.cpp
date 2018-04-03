//
// Created by qiguo on 2/1/18.
//
#include "proxy.h"

using namespace std;
using namespace tinynet;
using namespace tinyrpc;

void print_usage(const char * prog)
{
    printf("Usage: %s <config file>\n", prog);
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        exit(1);
    }
    string filename(argv[1]);
    Config config(filename);
    Proxy app(config);
    app.start();
    return 0;
}