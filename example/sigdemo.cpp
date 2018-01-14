//
// Created by qiguo on 1/14/18.
//

#include <iostream>
#include <cstdlib>
#include "sig.h"

using namespace tinynet;
using namespace std;

int main()
{
    Signal::signal(SIGUSR2, [](int sig){
        info("receive a SIGUSR2 signal");
        exit(0);
    });

    auto sigset = {SIGINT, SIGUSR1};

    Signal::signal(sigset, [](int sig){
        switch(sig)
        {
            case SIGINT:
                info("receive a SIGKILL signal");
                Signal::remove(SIGINT);
                break;
            case SIGUSR1:
                info("receive a SIGUSR1 signal");
                Signal::ignore(SIGUSR1);
                break;
            default:
                info("unexpected signal %d", sig);
        }
    });

    while (true) {}

    return 0;
}

