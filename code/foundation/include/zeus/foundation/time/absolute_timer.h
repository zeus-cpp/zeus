#pragma once
#include <string>
#include <functional>
#include <memory>
#include <ctime>

namespace zeus
{
class AbsoluteTimerImpl;
class AbsoluteTimer
{
public:
    AbsoluteTimer(bool automatic = true);
    ~AbsoluteTimer();
    AbsoluteTimer(const AbsoluteTimer&)            = delete;
    AbsoluteTimer(AbsoluteTimer&&)                 = delete;
    AbsoluteTimer& operator=(const AbsoluteTimer&) = delete;

    size_t AddAbsoluteTimerTask(const std::function<void()>& callback, tm target);
    bool   UpdateTimerTaskTarget(size_t id, tm target);
    bool   RemoveTimerTask(size_t id, bool wait = true);
    void   Start();
    void   Stop();
    void   SetThreadName(const std::string& name);
    void   SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback);
private:
    std::unique_ptr<AbsoluteTimerImpl> _impl;
};
}

#include "zeus/foundation/core/zeus_compatible.h"
