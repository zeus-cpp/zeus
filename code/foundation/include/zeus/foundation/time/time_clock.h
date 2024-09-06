#pragma once

#include <chrono>
#include <memory>

namespace zeus
{
class TimeClock
{
public:
    TimeClock();
    virtual ~TimeClock();
    TimeClock(const TimeClock&)                                  = delete;
    TimeClock(TimeClock&&)                                       = delete;
    TimeClock&                       operator=(const TimeClock&) = delete;
    virtual std::chrono::nanoseconds Elapsed() const             = 0;
    virtual void                     Reset()                     = 0;
};
}
#include "zeus/foundation/core/zeus_compatible.h"
