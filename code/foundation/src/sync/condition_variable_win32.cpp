#include "zeus/foundation/sync/condition_variable.h"
#ifdef _WIN32
#include <mutex>
#include <functional>
#include <thread>
#include <cassert>
#include <Windows.h>
using namespace std::chrono;
namespace zeus
{

struct ConditionVariableImpl
{
    std::thread::id    lockThread; //此变量用于调试检查
    CRITICAL_SECTION   mutex;
    CONDITION_VARIABLE condition;
};

ConditionVariable::ConditionVariable() : _impl(std::make_unique<ConditionVariableImpl>())
{
    InitializeCriticalSection(&_impl->mutex);
    InitializeConditionVariable(&_impl->condition);
}
ConditionVariable::~ConditionVariable()
{
    DeleteCriticalSection(&_impl->mutex);
}
void ConditionVariable::Wait(const std::function<bool()>& predicate)
{
    assert(_impl->lockThread == std::this_thread::get_id());
    while (!predicate())
    {
        SleepConditionVariableCS(&_impl->condition, &_impl->mutex, INFINITE);
    }
}

bool ConditionVariable::WaitTimeout(const std::chrono::steady_clock::duration& duration, const std::function<bool()>& predicate)
{
    assert(_impl->lockThread == std::this_thread::get_id());
    auto  begin        = steady_clock::now();
    DWORD waitDuration = static_cast<DWORD>(duration_cast<milliseconds>(duration).count());
    while (!predicate())
    {
        if (!SleepConditionVariableCS(&_impl->condition, &_impl->mutex, waitDuration))
        {
            return false;
        }
        auto now = steady_clock::now();
        if (now - begin >= duration)
        {
            return false;
        }
        else
        {
            waitDuration = duration_cast<milliseconds>(begin + duration - now).count();
        }
    }
    return true;
}

bool ConditionVariable::WaitUntil(const std::chrono::steady_clock::time_point& point, const std::function<bool()>& predicate)
{
    assert(_impl->lockThread == std::this_thread::get_id());
    auto duration = (point - steady_clock::now());
    return WaitTimeout(duration, predicate);
}

void ConditionVariable::NotifyOne()
{
    assert(_impl->lockThread == std::this_thread::get_id());
    WakeConditionVariable(&_impl->condition);
}
void ConditionVariable::NotifyAll()
{
    assert(_impl->lockThread == std::this_thread::get_id());
    WakeAllConditionVariable(&_impl->condition);
}

void ConditionVariable::Lock()
{
    EnterCriticalSection(&_impl->mutex);
    _impl->lockThread = std::this_thread::get_id();
}
void ConditionVariable::Unlock()
{
    _impl->lockThread = std::thread::id();
    LeaveCriticalSection(&_impl->mutex);
}
bool ConditionVariable::TryLock()
{
    bool lock = TryEnterCriticalSection(&_impl->mutex);
    if (lock)
    {
        _impl->lockThread = std::this_thread::get_id();
    }
    return lock;
}
} // namespace zeus
#endif