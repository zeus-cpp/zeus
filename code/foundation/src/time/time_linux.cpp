#include "zeus/foundation/time/time.h"
#ifdef __linux__
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <errno.h>
#include "zeus/foundation/time/time_utils.h"

namespace zeus
{
void Sleep(const std::chrono::steady_clock::duration& duration)
{
    using namespace std::chrono;
    timespec time      = DurationToTimeSpec(duration);
    timespec remaining = {};
    while (nanosleep(&time, &remaining) != 0 && EINTR == errno)
    {
        time = remaining;
    }
}

std::tm Localtime(std::time_t time)
{
    struct std::tm tm = {};
    localtime_r(&time, &tm);
    return tm;
}
bool Localtime(std::time_t time, std::tm& tm)
{
    return nullptr != localtime_r(&time, &tm);
}
std::time_t GetTimeZone()
{
    time_t utc      = 0;
    int    timeZone = 0;

    auto tm  = Localtime(utc); //转成当地时间
    timeZone = (tm.tm_hour > 12) ? (tm.tm_hour -= 24) : tm.tm_hour;
    return timeZone * 3600;
}
std::tm Gmtime(std::time_t time)
{
    struct std::tm tm = {};
    gmtime_r(&time, &tm);
    return tm;
}
bool Gmtime(std::time_t time, std::tm& tm)
{
    return nullptr != gmtime_r(&time, &tm);
}

} // namespace zeus
#endif