#include "relative_timer_impl.h"
#include "relative_timer_task.h"
using namespace std::chrono;
namespace zeus
{
RelativeTimerImpl::RelativeTimerImpl(bool automatic) : BaseTimer(automatic)
{
}
RelativeTimerImpl::~RelativeTimerImpl()
{
}
size_t RelativeTimerImpl::AddPeriodTimerTask(
    const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount

)
{
    auto task = std::make_shared<RelativeTimerTask>(GenerateId(), steady_clock::now(), callback, period, false, executeCount);
    return AddTimerTask(task);
}

size_t RelativeTimerImpl::AddPrecisePeriodTimerTask(
    const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
)
{
    auto task = std::make_shared<RelativeTimerTask>(GenerateId(), steady_clock::now(), callback, period, true, executeCount);
    return AddTimerTask(task);
}

size_t RelativeTimerImpl::AddDelayTimerTask(const std::function<void()>& callback, const std::chrono::steady_clock::duration& delay)
{
    auto delayTask = [callback](size_t /*count*/)
    {
        callback();
        return false;
    };
    auto task = std::make_shared<RelativeTimerTask>(GenerateId(), steady_clock::now(), delayTask, delay, false, 1);
    return AddTimerTask(task);
}

bool RelativeTimerImpl::UpdateTimerTaskPeriod(size_t id, const std::chrono::steady_clock::duration& period)
{
    auto task  = std::dynamic_pointer_cast<RelativeTimerTask>(GetQueueTask(id));
    bool state = false;
    if (task)
    {
        state = task->UpdatePeriod(period);
        UpdateQueueTask(task, true);
    }
    return state;
}

std::chrono::nanoseconds::rep RelativeTimerImpl::Now() const
{
    return steady_clock::now().time_since_epoch().count();
}

void RelativeTimerImpl::Wait(std::chrono::nanoseconds::rep now, std::chrono::nanoseconds::rep end)
{
    BaseTimer::Wait(now, end);
}
} // namespace zeus
