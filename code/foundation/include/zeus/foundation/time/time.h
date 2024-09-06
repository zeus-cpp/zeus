#pragma once
#include <chrono>
#include <ctime>

namespace zeus
{
//重新定义Sleep是因为，this_thread::sleep的实现依赖系统时间，受修改系统时间影响
void Sleep(const std::chrono::steady_clock::duration& duration);

//线程安全的localtime
std::tm Localtime(std::time_t time);

bool Localtime(std::time_t time, std::tm& tm);

//时区的偏移秒数,注意偏移有可能是负数 local=utc+偏移
std::time_t GetTimeZone();

//线程安全的gmtime
std::tm Gmtime(std::time_t time);

bool Gmtime(std::time_t time, std::tm& tm);

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
