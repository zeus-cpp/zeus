#include "zeus/foundation/time/high_time_clock.h"

namespace zeus
{

struct HighTimeClockImpl
{
    std::chrono::high_resolution_clock::time_point clock = std::chrono::high_resolution_clock::now();
};
HighTimeClock::HighTimeClock() : _impl(std::make_unique<HighTimeClockImpl>())
{
}
HighTimeClock::~HighTimeClock()
{
}
std::chrono::nanoseconds HighTimeClock::Elapsed() const
{
    return std::chrono::high_resolution_clock::now() - _impl->clock;
}
void HighTimeClock::Reset()
{
    _impl->clock = std::chrono::high_resolution_clock::now();
}
}