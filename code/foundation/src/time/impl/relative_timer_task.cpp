#include "relative_timer_task.h"
#include <cassert>

using namespace std::chrono;

namespace zeus
{

RelativeTimerTask::RelativeTimerTask(
    size_t id, const std::chrono::steady_clock::time_point& start, const std::function<bool(size_t count)>& callback,
    const std::chrono::steady_clock::duration& period, bool precise, size_t maxCount
)
    : BaseTimerTask(id), _last(start), _period(period), _precise(precise), _callback(callback), _maxCount(maxCount)
{
    _end = (_last + _period).time_since_epoch().count();
    assert(_callback);
}
RelativeTimerTask::~RelativeTimerTask()
{
}

bool RelativeTimerTask::UpdatePeriod(const std::chrono::steady_clock::duration& period)
{
    //精确任务不允许修改周期
    if (_precise)
    {
        return false;
    }
    std::unique_lock lock(*this);
    if (!IsEnable())
    {
        return false;
    }
    //周期任务
    _period = period;
    _end    = (_last + _period).time_since_epoch().count();
    return true;
}

std::chrono::nanoseconds::rep RelativeTimerTask::End()
{
    return _end;
}

bool RelativeTimerTask::Emit(const std::chrono::nanoseconds::rep& now, const std::function<void(const std::exception& exception)>& exceptionCallback)
{
    std::unique_lock lock(*this);
    if (!IsEnable())
    {
        return false;
    }
    _last = steady_clock::time_point(steady_clock::duration(now));
    ++_count;
    assert(_callback);
    bool terminate = false;
    try
    {
        terminate = !_callback(_count);
    }
    catch (const std::exception& e)
    {
        if (exceptionCallback)
        {
            exceptionCallback(e);
        }
    }

    if (terminate || (_maxCount && _count >= _maxCount))
    {
        DisableImpl();
        return false;
    }
    else
    {
        //周期性任务
        if (_precise)
        {
            // 此处保证:
            // 1. 下次执行的时间大于当前时间，即 _end > now
            // 2. 任意两次执行间隔都是 _period 的整数倍

            auto const n = steady_clock::duration(now - _end) / _period + 1;
            _end += (_period * n).count();
        }
        else
        {
            _end = (_last + _period).time_since_epoch().count();
        }
        return true;
    }
}

void RelativeTimerTask::DisableImpl()
{
    _callback = nullptr;
    BaseTimerTask::DisableImpl();
}

} // namespace zeus
