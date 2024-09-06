#pragma once

#include "zeus/foundation/time/time_clock.h"

namespace zeus
{
struct SteadyTimeClockImpl;
class SteadyTimeClock : public TimeClock
{
public:
    SteadyTimeClock();
    ~SteadyTimeClock();
    SteadyTimeClock(const SteadyTimeClock&)                    = delete;
    SteadyTimeClock(SteadyTimeClock&&)                         = delete;
    SteadyTimeClock&         operator=(const SteadyTimeClock&) = delete;
    std::chrono::nanoseconds Elapsed() const override;
    void                     Reset() override;
private:
    std::unique_ptr<SteadyTimeClockImpl> _impl;
};
}
#include "zeus/foundation/core/zeus_compatible.h"
