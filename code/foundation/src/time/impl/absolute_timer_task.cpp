#include "absolute_timer_task.h"
#include <cassert>

namespace zeus
{
AbsoluteTimerTask::AbsoluteTimerTask(
    size_t id, const std::chrono::system_clock::time_point& start, const std::function<void()>& callback,
    const std::chrono::system_clock::time_point& target
)
    : BaseTimerTask(id), _start(start), _end(target.time_since_epoch().count()), _callback(callback)
{
}
AbsoluteTimerTask::~AbsoluteTimerTask()
{
}
bool AbsoluteTimerTask::UpdateTarget(const std::chrono::system_clock::time_point& target)
{
    std::unique_lock lock(*this);
    if (!IsEnable())
    {
        return false;
    }
    _end = target.time_since_epoch().count();
    return true;
}
std::chrono::nanoseconds::rep AbsoluteTimerTask::End()
{
    return _end;
}
bool AbsoluteTimerTask::Emit(
    const std::chrono::nanoseconds::rep& /*now*/, const std::function<void(const std::exception& exception)>& exceptionCallback
)
{
    std::unique_lock lock(*this);
    if (!IsEnable())
    {
        return false;
    }
    assert(_callback);

    try
    {
        _callback();
    }
    catch (const std::exception& e)
    {
        if (exceptionCallback)
        {
            exceptionCallback(e);
        }
    }
    DisableImpl();
    return false;
}

void AbsoluteTimerTask::DisableImpl()
{
    _callback = nullptr;
    BaseTimerTask::DisableImpl();
}

} // namespace zeus
