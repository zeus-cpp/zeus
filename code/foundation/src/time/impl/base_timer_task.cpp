#include "base_timer_task.h"

namespace zeus
{

BaseTimerTask::BaseTimerTask(size_t id) : _id(id)
{
}

BaseTimerTask::~BaseTimerTask()
{
}

size_t BaseTimerTask::Id() const
{
    return _id;
}

void BaseTimerTask::Disable()
{
    std::unique_lock lock(*this);
    DisableImpl();
}

void BaseTimerTask::lock()
{
    _mutex.lock();
}
void BaseTimerTask::unlock()
{
    _mutex.unlock();
}

std::chrono::nanoseconds::rep BaseTimerTask::QueuePoint() const
{
    return _queuePoint;
}

void BaseTimerTask::SetQueuePoint(const std::chrono::nanoseconds::rep& point)
{
    _queuePoint = point;
}

bool BaseTimerTask::IsEnable() const
{
    return _enable;
}

void BaseTimerTask::DisableImpl()
{
    _enable = false;
}

} // namespace zeus