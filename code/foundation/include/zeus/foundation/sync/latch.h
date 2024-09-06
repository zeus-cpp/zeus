#pragma once

#include <chrono>
#include <memory>

namespace zeus
{
struct LatchImpl;
class Latch
{
public:
    Latch(size_t count);

    /// Destructor
    /// Precondition: No threads are waiting or invoking count_down on @c *this.

    ~Latch();
    Latch(const Latch&)            = delete;
    Latch(Latch&&)                 = delete;
    Latch& operator=(const Latch&) = delete;

    /// Blocks until the latch has counted down to zero.
    void Wait();

    bool WaitTimeout(const std::chrono::nanoseconds& duration);

    bool WaitUntil(const std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration>& point);

    void CountDown();

    void Reset();

private:
    std::unique_ptr<LatchImpl> _impl;
};
}

#include "zeus/foundation/core/zeus_compatible.h"
