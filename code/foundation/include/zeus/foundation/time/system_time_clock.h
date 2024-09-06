#pragma once

#include "zeus/foundation/time/time_clock.h"

namespace zeus
{
struct SystemTimeClockImpl;
class SystemTimeClock : public TimeClock

{
public:
    SystemTimeClock();
    ~SystemTimeClock();
    SystemTimeClock(const SystemTimeClock&)                    = delete;
    SystemTimeClock(SystemTimeClock&&)                         = delete;
    SystemTimeClock&         operator=(const SystemTimeClock&) = delete;
    std::chrono::nanoseconds Elapsed() const override;
    void                     Reset() override;
private:
    std::unique_ptr<SystemTimeClockImpl> _impl;
};
}
#include "zeus/foundation/core/zeus_compatible.h"
