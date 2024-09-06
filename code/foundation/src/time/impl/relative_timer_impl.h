#pragma once

#include <functional>
#include "base_timer.h"
#include "relative_timer_task.h"
namespace zeus
{
class RelativeTimerImpl : public BaseTimer
{
public:
    RelativeTimerImpl(bool automatic);
    ~RelativeTimerImpl() override;
    size_t AddPeriodTimerTask(
        const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
    );
    size_t AddPrecisePeriodTimerTask(
        const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount
    );
    size_t                        AddDelayTimerTask(const std::function<void()>& callback, const std::chrono::steady_clock::duration& delay);
    bool                          UpdateTimerTaskPeriod(size_t id, const std::chrono::steady_clock::duration& period);
    std::chrono::nanoseconds::rep Now() const override;
private:
    void Wait(std::chrono::nanoseconds::rep now, std::chrono::nanoseconds::rep end) override;
};
}
