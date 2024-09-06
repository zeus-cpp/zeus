#include "zeus/foundation/time/system_time_clock.h"

namespace zeus
{

struct SystemTimeClockImpl
{
    std::chrono::system_clock::time_point clock = std::chrono::system_clock::now();
};
SystemTimeClock::SystemTimeClock() : _impl(std::make_unique<SystemTimeClockImpl>())
{
}
SystemTimeClock::~SystemTimeClock()
{
}
std::chrono::nanoseconds SystemTimeClock::Elapsed() const
{
    return std::chrono::system_clock::now() - _impl->clock;
}
void SystemTimeClock::Reset()
{
    _impl->clock = std::chrono::system_clock::now();
}
}