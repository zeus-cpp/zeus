#include "zeus/foundation/time/steady_time_clock.h"

namespace zeus
{

struct SteadyTimeClockImpl
{
    std::chrono::steady_clock::time_point clock = std::chrono::steady_clock::now();
};
SteadyTimeClock::SteadyTimeClock() : _impl(std::make_unique<SteadyTimeClockImpl>())
{
}
SteadyTimeClock::~SteadyTimeClock()
{
}
std::chrono::nanoseconds SteadyTimeClock::Elapsed() const
{
    return std::chrono::steady_clock::now() - _impl->clock;
}
void SteadyTimeClock::Reset()
{
    _impl->clock = std::chrono::steady_clock::now();
}
}