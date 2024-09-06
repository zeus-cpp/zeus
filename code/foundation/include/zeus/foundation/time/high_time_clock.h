#pragma once

#include "zeus/foundation/time/time_clock.h"

namespace zeus
{
struct HighTimeClockImpl;
class HighTimeClock : public TimeClock

{
public:
    HighTimeClock();
    ~HighTimeClock();
    HighTimeClock(const HighTimeClock&)                      = delete;
    HighTimeClock(HighTimeClock&&)                           = delete;
    HighTimeClock&           operator=(const HighTimeClock&) = delete;
    std::chrono::nanoseconds Elapsed() const override;
    void                     Reset() override;
private:
    std::unique_ptr<HighTimeClockImpl> _impl;
};
}
#include "zeus/foundation/core/zeus_compatible.h"
