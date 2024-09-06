#include "zeus/foundation/time/absolute_timer.h"
#include "impl/absolute_timer_impl.h"
namespace zeus
{
AbsoluteTimer::AbsoluteTimer(bool automatic) : _impl(std::make_unique<AbsoluteTimerImpl>(automatic))
{
}
AbsoluteTimer::~AbsoluteTimer()
{
}

size_t AbsoluteTimer::AddAbsoluteTimerTask(const std::function<void()>& callback, tm target)
{
    return _impl->AddAbsoluteTimerTask(callback, target);
}
bool AbsoluteTimer::UpdateTimerTaskTarget(size_t id, tm target)
{
    return _impl->UpdateTimerTaskTarget(id, target);
}
bool AbsoluteTimer::RemoveTimerTask(size_t id, bool wait)
{
    return _impl->RemoveTimerTask(id, wait);
}
void AbsoluteTimer::Start()
{
    _impl->Start();
}
void AbsoluteTimer::Stop()
{
    _impl->Stop();
}

void AbsoluteTimer::SetThreadName(const std::string& name)
{
    _impl->SetThreadName(name);
}
void AbsoluteTimer::SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback)
{
    _impl->SetExceptionCallcack(callback);
}
} // namespace zeus
