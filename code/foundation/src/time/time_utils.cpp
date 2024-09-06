#include "zeus/foundation/time/time_utils.h"
#include <sstream>
#include <iomanip>
#include <cassert>
#include "zeus/foundation/time/time.h"

namespace zeus
{
std::string GetCurrentDateTimeString()
{
    using std::setfill;
    using std::setw;

    std::stringstream result;
    result.imbue(std::locale::classic());
    const auto    clock = std::chrono::system_clock::now();
    const auto    now   = std::chrono::system_clock::to_time_t(clock);
    const auto    msecs = clock.time_since_epoch().count() % 10000000;
    const std::tm time  = zeus::Localtime(now);
    result << setfill('0') << setw(4) << 1900 + time.tm_year << '-' << setfill('0') << setw(2) << 1 + time.tm_mon << '-' << setfill('0') << setw(2)
           << time.tm_mday << ' ' << setfill('0') << setw(2) << time.tm_hour << ':' << setfill('0') << setw(2) << time.tm_min << ':' << setfill('0')
           << setw(2) << time.tm_sec << "." << setfill('0') << setw(3) << msecs / 10000;
    return result.str();
}

std::string FormatSystemClockTime(const std::chrono::system_clock::time_point& point, const std::string& format)
{
    const auto time = std::chrono::system_clock::to_time_t(point);
    return FormatTime(time, format);
}

std::string FormatTime(std::time_t time, const std::string& format, const std::locale& locale)
{
    std::stringstream stream;
    stream.imbue(locale);
    const auto localedTime = Localtime(time);
    stream << std::put_time(&localedTime, format.c_str());
    return stream.str();
}

Duration FormatDuration(const std::chrono::steady_clock::duration& duration)
{
    const auto millTime = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    static constexpr auto kHourPerDay      = 24;
    static constexpr auto kMinutePerHour   = 60;
    static constexpr auto kSecondPerMinute = 60;
    static constexpr auto kMillPerSecond   = 1000;
    Duration              result           = {};
    result.day                             = millTime.count() / kMillPerSecond / kSecondPerMinute / kMinutePerHour / kHourPerDay;
    result.hour                            = millTime.count() / kMillPerSecond / kSecondPerMinute / kMinutePerHour % kHourPerDay;
    result.minute                          = millTime.count() / kMillPerSecond / kSecondPerMinute % kMinutePerHour;
    result.second                          = millTime.count() / kMillPerSecond % kSecondPerMinute;
    result.millisecond                     = millTime.count() % kMillPerSecond;
    return result;
}

std::string FormatDurationStr(const std::chrono::steady_clock::duration& duration)
{
    const auto        time = FormatDuration(duration);
    std::stringstream stream;
    stream.imbue(std::locale::classic());
    stream << time.day << "-";
    stream << std::setw(2) << std::setfill('0') << time.hour << ":";
    stream << std::setw(2) << std::setfill('0') << time.minute << ":";
    stream << std::setw(2) << std::setfill('0') << time.second << ".";
    stream << std::setw(3) << std::setfill('0') << time.millisecond;
    return stream.str();
}

std::chrono::steady_clock::duration TimeSpecToDuration(const ::timespec& time)
{
    return std::chrono::seconds(time.tv_sec) + std::chrono::nanoseconds(time.tv_nsec);
}

::timespec DurationToTimeSpec(const std::chrono::steady_clock::duration& duration)
{
    using namespace std::chrono;
    ::timespec time   = {};
    auto       second = duration_cast<seconds>(duration);
    time.tv_sec       = second.count();
    time.tv_nsec      = duration_cast<nanoseconds>(duration - second).count();
    assert(IsValidTimeSpec(time));
    return time;
}

::timespec TimeSpecAdd(const ::timespec& time, const std::chrono::steady_clock::duration& duration)
{
    return DurationToTimeSpec(TimeSpecToDuration(time) + duration);
}

::timespec TimeSpecAdd(const ::timespec& time, const ::timespec& duration)
{
    return DurationToTimeSpec(TimeSpecToDuration(time) + TimeSpecToDuration(duration));
}

bool IsValidTimeSpec(const ::timespec& time)
{
    return (time.tv_nsec < 1000000000) && (time.tv_sec >= 0);
}

std::string FormatDurationStr(const std::chrono::steady_clock::duration& duration, const std::string& delim)
{
    const auto        time = zeus::FormatDuration(duration);
    std::stringstream stream;
    stream.imbue(std::locale::classic());
    bool pre = false;
    if (time.day)
    {
        if (pre)
        {
            stream << delim;
        }
        stream << time.day << "days";
        pre = true;
    }
    if (time.hour)
    {
        if (pre)
        {
            stream << delim;
        }
        stream << time.hour << "hours";
        pre = true;
    }
    if (time.minute)
    {
        if (pre)
        {
            stream << delim;
        }
        stream << time.minute << "minutes";
        pre = true;
    }
    if (time.second)
    {
        if (pre)
        {
            stream << delim;
        }
        stream << time.second << "seconds";
    }
    return stream.str();
}
} // namespace zeus