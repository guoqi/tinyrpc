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

pthread_mutex_t mtx;

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
        pthread_mutex_lock(&mtx);
        cout << q.front() << endl;
        pthread_mutex_unlock(&mtx);
        q.pop();
    }
}

int main()
{
    pthread_mutex_init(&mtx, nullptr);

    vector<pthread_t> threads;
    for (int i=0; i<100; ++i)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, &input, nullptr);
        threads.push_back(thread);
    }

    for (int i=0; i<4; ++i)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, &output, nullptr);
    }

    for(auto & thread : threads)
    {
        pthread_join(thread, nullptr);
    }

    sleep(5);

    pthread_mutex_destroy(&mtx);
    //output(nullptr);

    return 0;
}