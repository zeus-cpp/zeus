#include "zeus/foundation/time/relative_timer.h"
#include "impl/relative_timer_impl.h"
namespace zeus
{
RelativeTimer::RelativeTimer(bool automatic) : _impl(std::make_unique<RelativeTimerImpl>(automatic))
{
}
RelativeTimer::~RelativeTimer()
{
}

size_t RelativeTimer::AddPeriodTimerTask(
    const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
)
{
    return _impl->AddPeriodTimerTask(callback, period, executeCount);
}

size_t RelativeTimer::AddSimplePeriodTimerTask(
    const std::function<void()>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
)
{
    auto wrapper = [callback](size_t)
    {
        callback();
        return true;
    };
    return _impl->AddPeriodTimerTask(wrapper, period, executeCount);
}

size_t RelativeTimer::AddPrecisePeriodTimerTask(
    const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
)
{
    return _impl->AddPrecisePeriodTimerTask(callback, period, executeCount);
}

size_t RelativeTimer::AddSimplePrecisePeriodTimerTask(
    const std::function<void()>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
)
{
    auto wrapper = [callback](size_t)
    {
        callback();
        return true;
    };
    return _impl->AddPrecisePeriodTimerTask(wrapper, period, executeCount);
}

size_t RelativeTimer::AddDelayTimerTask(const std::function<void()>& callback, const std::chrono::steady_clock::duration& delay)
{
    return _impl->AddDelayTimerTask(callback, delay);
}
bool RelativeTimer::UpdateTimerTaskPeriod(size_t id, const std::chrono::steady_clock::duration& period)
{
    return _impl->UpdateTimerTaskPeriod(id, period);
}
bool RelativeTimer::RemoveTimerTask(size_t id, bool wait)
{
    return _impl->RemoveTimerTask(id, wait);
}
void RelativeTimer::Start()
{
    _impl->Start();
}
void RelativeTimer::Stop()
{
    _impl->Stop();
}

void RelativeTimer::SetThreadName(const std::string& name)
{
    _impl->SetThreadName(name);
}
void RelativeTimer::SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback)
{
    _impl->SetExceptionCallcack(callback);
}
} // namespace zeus
