#pragma once
#include <ctime>
#include <functional>
#include "base_timer.h"
#include "absolute_timer_task.h"
namespace zeus
{
class AbsoluteTimerImpl : public BaseTimer
{
public:
    AbsoluteTimerImpl(bool automatic);
    ~AbsoluteTimerImpl() override;
    size_t                        AddAbsoluteTimerTask(const std::function<void()>& callback, tm target);
    bool                          UpdateTimerTaskTarget(size_t id, tm target);
    std::chrono::nanoseconds::rep Now() const override;

private:
    void Wait(std::chrono::nanoseconds::rep now, std::chrono::nanoseconds::rep end) override;
};
}
