#pragma once
#include <functional>
#include <chrono>
#include <memory>
namespace zeus
{
struct TimeTickimpl;
class TimeTick
{
public:
    TimeTick(const std::function<void(const std::chrono::steady_clock::duration& duration)>& logCallback);
    TimeTick(std::function<void(const std::chrono::steady_clock::duration& duration)>&& logCallback);
    ~TimeTick();

    TimeTick(const TimeTick&)            = delete;
    TimeTick& operator=(const TimeTick&) = delete;
    TimeTick(TimeTick&& other) noexcept;
    TimeTick& operator=(TimeTick&& other) noexcept;

private:
    std::unique_ptr<TimeTickimpl> _impl;
};
}

#include "zeus/foundation/core/zeus_compatible.h"
