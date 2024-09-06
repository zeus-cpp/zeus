#include "zeus/foundation/time/time.h"

#ifdef _WIN32
#include <ctime>
#include <Windows.h>
namespace zeus
{

void Sleep(const std::chrono::steady_clock::duration& duration)
{
    ::Sleep(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}
std::tm Localtime(std::time_t time)
{
    struct std::tm tm = {};
    localtime_s(&tm, &time);
    return tm;
}
bool Localtime(std::time_t time, std::tm& tm)
{
    return 0 == localtime_s(&tm, &time);
}
std::time_t GetTimeZone()
{
    TIME_ZONE_INFORMATION tz = {};
    GetTimeZoneInformation(&tz); //时区偏差：min
    return -tz.Bias * 60;        //秒
}
std::tm Gmtime(std::time_t time)
{
    struct std::tm tm = {};
    gmtime_s(&tm, &time);
    return tm;
}
bool Gmtime(std::time_t time, std::tm& tm)
{
    return 0 == gmtime_s(&tm, &time);
}
} // namespace zeus
#endif
