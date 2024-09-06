#pragma once
#include <functional>
#include <atomic>
#include "base_timer_task.h"
namespace zeus
{
class AbsoluteTimerTask : public BaseTimerTask
{
public:
    AbsoluteTimerTask(
        size_t id, const std::chrono::system_clock::time_point& start, const std::function<void()>& callback,
        const std::chrono::system_clock::time_point& target
    );
    ~AbsoluteTimerTask() override;
    bool                          UpdateTarget(const std::chrono::system_clock::time_point& target);
    std::chrono::nanoseconds::rep End() override;
    bool Emit(const std::chrono::nanoseconds::rep& now, const std::function<void(const std::exception& exception)>& exceptionCallback) override;
protected:
    void DisableImpl() override;
private:
    const std::chrono::system_clock::time_point _start;
    std::atomic<std::chrono::nanoseconds::rep>  _end;
    std::function<void()>                       _callback;
};
}
