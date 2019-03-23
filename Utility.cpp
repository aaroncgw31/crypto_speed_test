#include "Utility.h"

#include <stdlib.h>
#include <time.h>

int GetDate(int64_t ts)
{
    int64_t seconds = ts/1000000000LL;
    struct tm t;
    localtime_r(&seconds, &t);

    return (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
}

int GetToday()
{
    return GetDate(GetNow());
}

int64_t GetNow()
{
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);

    return ( (int64_t)ts.tv_sec ) * 1000000000LL + ts.tv_nsec;
}

std::string GetNowStr()
{
    return FormatTime(GetNow());
}

std::string FormatTime(int64_t ts)
{
    int64_t seconds = ts / 1000000000LL;
    int64_t nanos = ts % 1000000000LL;

    struct tm lt;
    ::localtime_r(&seconds, &lt);

    char time_buffer[256];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &lt);

    char ret[256];
    snprintf(ret, sizeof(ret), "%s.%09ld", time_buffer, nanos);

    return ret;
}

int64_t ParseTime(const char* time)
{

}
