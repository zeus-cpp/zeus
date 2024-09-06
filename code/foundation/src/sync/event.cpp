#include "zeus/foundation/sync/event.h"
#include <mutex>
#include "zeus/foundation/sync/condition_variable.h"

using namespace std::chrono;
namespace zeus
{
struct EventImpl
{
    ConditionVariable condition;
    bool              flag;
    bool              allFlag;
};
Event::Event() : _impl(std::make_unique<EventImpl>())
{
}
Event::~Event()
{
}
void Event::Reset()
{
    std::unique_lock lock(_impl->condition);
    _impl->flag    = false;
    _impl->allFlag = false;
}

void Event::Wait()
{
    std::unique_lock lock(_impl->condition);
    _impl->condition.Wait([this]() { return _impl->flag || _impl->allFlag; });
    _impl->flag = false;
}
bool Event::WaitTimeout(const std::chrono::steady_clock::duration& duration)
{
    std::unique_lock lock(_impl->condition);
    bool             state = _impl->condition.WaitTimeout(duration, [this]() { return _impl->flag || _impl->allFlag; });
    if (state)
    {
        _impl->flag = false;
    }
    return state;
}
bool Event::WaitUntil(const std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration>& point)
{
    std::unique_lock lock(_impl->condition);
    bool             state = _impl->condition.WaitUntil(point, [this]() { return _impl->flag || _impl->allFlag; });
    if (state)
    {
        _impl->flag = false;
    }
    return state;
}
void Event::Notify()
{
    std::unique_lock lock(_impl->condition);
    _impl->flag = true;
    _impl->condition.NotifyOne();
}
void Event::NotifyAll()
{
    std::unique_lock lock(_impl->condition);
    _impl->allFlag = true;
    _impl->condition.NotifyAll();
}
} // namespace zeus
