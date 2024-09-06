#include "zeus/foundation/sync/latch.h"
#include <mutex>
#include "zeus/foundation/sync/condition_variable.h"

using namespace std::chrono;

namespace zeus
{
struct LatchImpl
{
    ConditionVariable condition;
    size_t            count = 0;
};
Latch::Latch(size_t count) : _impl(std::make_unique<LatchImpl>())
{
    _impl->count = count;
}
Latch::~Latch()
{
}
void Latch::Wait()
{
    std::unique_lock lock(_impl->condition);
    _impl->condition.Wait([this]() { return _impl->count == 0; });
}
bool Latch::WaitTimeout(const std::chrono::nanoseconds& duration)
{
    std::unique_lock lock(_impl->condition);
    return _impl->condition.WaitTimeout(duration, [this]() { return _impl->count == 0; });
}
bool Latch::WaitUntil(const std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration>& point)
{
    std::unique_lock lock(_impl->condition);
    return _impl->condition.WaitUntil(point, [this]() { return _impl->count == 0; });
}
void Latch::CountDown()
{
    std::unique_lock lock(_impl->condition);
    _impl->count--;
    _impl->condition.NotifyAll();
}
void Latch::Reset()
{
    std::unique_lock lock(_impl->condition);
    _impl->count = 0;
    _impl->condition.NotifyAll();
}
} // namespace zeus
