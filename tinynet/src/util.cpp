//
// Created by qiguo on 1/1/18.
//
#include <sys/time.h>
#include "util.h"

namespace util
{
    namespace Time
    {
        int64_t nowMs()
        {
            return nowUs() / 1000;
        }

        int64_t nowUs()
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);

            return tv.tv_sec * 1000000 + tv.tv_usec;
        }
    }
}
