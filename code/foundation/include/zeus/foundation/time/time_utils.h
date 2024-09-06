#pragma once
#include <string>
#include <chrono>
#include <locale>
#include <ctime>
#ifdef _WIN32
#include <zeus/foundation/core/win/win_windef.h>
#endif

namespace zeus
{

std::string GetCurrentDateTimeString();

//format格式字符串和std::put_time一致
std::string FormatSystemClockTime(const std::chrono::system_clock::time_point& point, const std::string& format);

//format格式字符串和std::put_time一致
std::string FormatTime(std::time_t time, const std::string& format, const std::locale& locale = std::locale::classic());

struct Duration
{
    size_t day;
    size_t hour;
    size_t minute;
    size_t second;
    size_t millisecond;
};
//时间段换算
Duration FormatDuration(const std::chrono::steady_clock::duration& duration);

//时间段打印形式 day-hour:min:second:millisecond 3-02:23:11.958
std::string FormatDurationStr(const std::chrono::steady_clock::duration& duration);

std::chrono::steady_clock::duration TimeSpecToDuration(const ::timespec& time);
::timespec                          DurationToTimeSpec(const std::chrono::steady_clock::duration& duration);
::timespec                          TimeSpecAdd(const ::timespec& time, const std::chrono::steady_clock::duration& duration);
::timespec                          TimeSpecAdd(const ::timespec& time, const timespec& duration);
bool                                IsValidTimeSpec(const ::timespec& time);
#ifdef _WIN32
std::chrono::system_clock::time_point FiletimeToSystemTime(const FILETIME& time);
std::chrono::system_clock::time_point FiletimeToSystemTime(const LARGE_INTEGER& time);
#endif
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
