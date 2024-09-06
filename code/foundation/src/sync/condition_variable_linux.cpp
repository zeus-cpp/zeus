#include "zeus/foundation/sync/condition_variable.h"
#ifdef __linux__
#include <functional>
#include <thread>
#include <cassert>
#include <pthread.h>
#include "zeus/foundation/time/time_utils.h"
#include "zeus/foundation/thread/thread_utils.h"

using namespace std::chrono;

namespace zeus
{
struct ConditionVariableImpl
{
    uint64_t        lockThread = 0; //此变量用于调试检查
    pthread_mutex_t mutex      = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t  condition  = PTHREAD_COND_INITIALIZER;
};

ConditionVariable::ConditionVariable() : _impl(std::make_unique<ConditionVariableImpl>())
{
    pthread_mutex_init(&_impl->mutex, nullptr);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&_impl->condition, &cattr);
}

ConditionVariable::~ConditionVariable()
{
    pthread_cond_destroy(&_impl->condition);
    pthread_mutex_destroy(&_impl->mutex);
}

void ConditionVariable::Wait(const std::function<bool()>& predicate)
{
    assert(_impl->lockThread == GetThreadId());
    while (!predicate())
    {
        pthread_cond_wait(&_impl->condition, &_impl->mutex);
    }
}
bool ConditionVariable::WaitTimeout(const std::chrono::steady_clock::duration& duration, const std::function<bool()>& predicate)
{
    assert(_impl->lockThread == GetThreadId());
    auto begin        = steady_clock::now();
    auto waitDuration = duration;
    while (!predicate())
    {
        timespec time = {};
        clock_gettime(CLOCK_MONOTONIC, &time);
        time              = TimeSpecAdd(time, waitDuration);
        const auto result = pthread_cond_timedwait(&_impl->condition, &_impl->mutex, &time);
        assert(ETIMEDOUT == result || 0 == result);
        if (ETIMEDOUT == result)
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
            waitDuration = begin + duration - now;
        }
    }
    return true;
}
bool ConditionVariable::WaitUntil(const std::chrono::steady_clock::time_point& point, const std::function<bool()>& predicate)
{
    assert(_impl->lockThread == GetThreadId());
    auto duration = (point - steady_clock::now());
    return WaitTimeout(duration, predicate);
}

void ConditionVariable::NotifyOne()
{
    assert(_impl->lockThread == GetThreadId());
    pthread_cond_signal(&_impl->condition);
}
void ConditionVariable::NotifyAll()
{
    assert(_impl->lockThread == GetThreadId());
    pthread_cond_broadcast(&_impl->condition);
}
void ConditionVariable::Lock()
{
    pthread_mutex_lock(&_impl->mutex);
    _impl->lockThread = GetThreadId();
}
void ConditionVariable::Unlock()
{
    _impl->lockThread = 0;
    pthread_mutex_unlock(&_impl->mutex);
}
bool ConditionVariable::TryLock()
{
    bool lock = (0 == pthread_mutex_trylock(&_impl->mutex));
    if (lock)
    {
        _impl->lockThread = GetThreadId();
    }
    return lock;
}

} // namespace zeus
#endif
