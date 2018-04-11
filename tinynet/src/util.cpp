//
// Created by qiguo on 1/1/18.
//
#include <sys/time.h>
#include <sstream>
#include "util.h"

using namespace std;

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

        // current datetime in xxxx-xx-xx xx:xx:xx format
        std::string datetime()
        {
            time_t tm = time(nullptr);
            char buf[32] = {0};

            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tm));

            return std::string(buf);
        }
    }

    std::string toHex(const char * data, size_t size)
    {
        stringstream ss;

        ss << "0x";

        for (int i=0; i<size; ++i)
        {
            char buff[16] = {0};
            snprintf(buff, sizeof(buff), "%x", *(data + i));
            ss << buff;
        }

        return ss.str();
    }

}
