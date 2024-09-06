#pragma once
#include <functional>
#include <atomic>
#include "base_timer_task.h"

namespace zeus
{
class RelativeTimerTask : public BaseTimerTask
{
public:
    RelativeTimerTask(
        size_t id, const std::chrono::steady_clock::time_point& start, const std::function<bool(size_t count)>& callback,
        const std::chrono::steady_clock::duration& period, bool precise, size_t maxCount
    );
    ~RelativeTimerTask() override;
    bool                          UpdatePeriod(const std::chrono::steady_clock::duration& period);
    std::chrono::nanoseconds::rep End() override;
    bool Emit(const std::chrono::nanoseconds::rep& now, const std::function<void(const std::exception& exception)>& exceptionCallback) override;
protected:
    void DisableImpl() override;
private:
    std::atomic<std::chrono::nanoseconds::rep> _end;
    std::chrono::steady_clock::time_point      _last;
    std::chrono::steady_clock::duration        _period;
    const bool                                 _precise;
    std::function<bool(size_t count)>          _callback;
    const size_t                               _maxCount;
    size_t                                     _count = 0;
};
}
