#include "zeus/foundation/time/time_tick.h"

namespace zeus
{
struct TimeTickimpl
{
    std::chrono::steady_clock::time_point                                    begin;
    std::function<void(const std::chrono::steady_clock::duration& duration)> logCallback;
};
TimeTick::TimeTick(const std::function<void(const std::chrono::steady_clock::duration& duration)>& logCallback)
    : _impl(std::make_unique<TimeTickimpl>())
{
    _impl->begin       = std::chrono::steady_clock::now();
    _impl->logCallback = logCallback;
}
TimeTick::TimeTick(std::function<void(const std::chrono::steady_clock::duration& duration)>&& logCallback) : _impl(std::make_unique<TimeTickimpl>())
{
    _impl->begin = std::chrono::steady_clock::now();
    _impl->logCallback.swap(logCallback);
}
TimeTick::~TimeTick()
{
    if (_impl && _impl->logCallback)
    {
        _impl->logCallback(std::chrono::steady_clock::now() - _impl->begin);
    }
}
TimeTick::TimeTick(TimeTick&& other) noexcept : _impl(std::move(other._impl))
{
}
TimeTick& TimeTick::operator=(TimeTick&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
} // namespace zeus
