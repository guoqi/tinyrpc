//
// Created by qiguo on 4/21/18.
//

#include "ts_queue.h"
#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>

using namespace tinyrpc;
using namespace std;

TSQueue<int> q;

void * input(void *)
{
    for (int i=0; i<1000; ++i)
    {
        q.push(i);
    }
}

void * output(void *)
{
    while (! q.empty())
    {
        cout << q.front() << endl;
        q.pop();
    }
}

int main()
{
    vector<pthread_t> threads;
    for (int i=0; i<2; ++i)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, &input, nullptr);
        threads.push_back(thread);
    }

    /*
    for (int i=0; i<4; ++i)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, &output, nullptr);
        threads.push_back(thread);
    }
    */

    for(auto & thread : threads)
    {
        pthread_join(thread, nullptr);
    }

    output(nullptr);

    return 0;
}