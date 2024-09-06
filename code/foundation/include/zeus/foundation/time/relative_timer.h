#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <memory>
namespace zeus
{
class RelativeTimerImpl;
class RelativeTimer
{
public:
    RelativeTimer(bool automatic = true);
    ~RelativeTimer();
    RelativeTimer(const RelativeTimer&)            = delete;
    RelativeTimer(RelativeTimer&&)                 = delete;
    RelativeTimer& operator=(const RelativeTimer&) = delete;
    //添加周期定时任务，executeCount表示执行次数，0表示无限次， 回调返回值表示是否继续执行下次任务，true表示继续执行，false表示不再执行，count表示执行次数
    size_t         AddPeriodTimerTask(
                const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount = 0
            );

    size_t AddSimplePeriodTimerTask(
        const std::function<void()>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount = 0
    );
    //添加精确周期定时任务，精确的意思是任务下次执行时间不会受到上次任务回调执行时间的影响
    size_t AddPrecisePeriodTimerTask(
        const std::function<bool(size_t count)>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount = 0
    );

    size_t AddSimplePrecisePeriodTimerTask(
        const std::function<void()>& callback, const std::chrono::steady_clock::duration& period, size_t executeCount = 0
    );
    //添加精确周期定时任务，精确的意思是任务下次执行时间不会受到上次任务回调执行时间的影响

    //添加延迟定时任务，延迟定时任务等于执行次数为1的周期定时任务
    size_t AddDelayTimerTask(const std::function<void()>& callback, const std::chrono::steady_clock::duration& delay);
    //更新周期定时任务和延迟定时任务的周期，返回值表示是否更新成功，true表示更新成功，false表示任务已经结束或者不允许更新，
    bool   UpdateTimerTaskPeriod(size_t id, const std::chrono::steady_clock::duration& period);
    bool   RemoveTimerTask(size_t id, bool wait = true);
    void   Start();
    void   Stop();
    void   SetThreadName(const std::string& name);
    void   SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback);
private:
    std::unique_ptr<RelativeTimerImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
