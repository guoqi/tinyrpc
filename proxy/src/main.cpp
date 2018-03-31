//
// Created by qiguo on 2/1/18.
//
#include "proxy.h"

using namespace std;
using namespace tinynet;
using namespace tinyrpc;

void print_usage()
{
    printf("Usage: ./tinyrpc <config file>\n");
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        print_usage();
        exit(1);
    }
    string filename(argv[1]);
    Config config(filename);
    Proxy app(config);
    app.start();
    return 0;
}