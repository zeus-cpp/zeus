#include "absolute_timer_impl.h"
#include <cassert>
#include "absolute_timer_task.h"

using namespace std::chrono;
namespace zeus
{
AbsoluteTimerImpl::AbsoluteTimerImpl(bool automatic) : BaseTimer(automatic)
{
}
AbsoluteTimerImpl::~AbsoluteTimerImpl()
{
}
size_t AbsoluteTimerImpl::AddAbsoluteTimerTask(const std::function<void()>& callback, tm target)
{
    const time_t time = std::mktime(&target);
    if (time != -1)
    {
        auto task = std::make_shared<AbsoluteTimerTask>(GenerateId(), system_clock::now(), callback, system_clock::from_time_t(time));
        return AddTimerTask(task);
    }
    return 0;
}

bool AbsoluteTimerImpl::UpdateTimerTaskTarget(size_t id, tm target)
{
    const time_t time = std::mktime(&target);
    if (-1 == time)
    {
        return false;
    }
    auto task  = std::dynamic_pointer_cast<AbsoluteTimerTask>(GetQueueTask(id));
    bool state = false;
    if (task)
    {
        state = task->UpdateTarget(system_clock::from_time_t(time));
        UpdateQueueTask(task, true);
    }
    return state;
}

std::chrono::nanoseconds::rep AbsoluteTimerImpl::Now() const
{
    return system_clock::now().time_since_epoch().count();
}

void AbsoluteTimerImpl::Wait(nanoseconds::rep now, nanoseconds::rep end)
{
    const auto threshold = seconds(1);
    assert(now < end);
    const auto duration = system_clock::duration(end - now);
    if (duration < threshold)
    {
        BaseTimer::Wait(now, end);
    }
    else
    {
        BaseTimer::Wait(now, now + (end - now) / 10);
    }
}
} // namespace zeus
