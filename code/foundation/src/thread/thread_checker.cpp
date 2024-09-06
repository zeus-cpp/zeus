#include "zeus/foundation/thread/thread_checker.h"
#include <thread>
#include <mutex>
#include "zeus/foundation/thread/thread_utils.h"

namespace zeus
{
struct ThreadCheckerImpl
{
    std::mutex mutex;
    uint64_t   threadId = 0;
    bool       attached = false;
};
ThreadChecker::ThreadChecker(bool attach) : _impl(std::make_unique<ThreadCheckerImpl>())
{
    if (attach)
    {
        _impl->threadId = GetThreadId();
        _impl->attached = true;
    }
}
ThreadChecker::~ThreadChecker()
{
}
bool ThreadChecker::IsCurrent() const
{
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (_impl->attached)
    {
        return GetThreadId() == _impl->threadId;
    }
    else
    {
        _impl->threadId = GetThreadId();
        _impl->attached = true;
        return true;
    }
}
void ThreadChecker::Detach()
{
    std::lock_guard<std::mutex> lock(_impl->mutex);
    _impl->attached = false;
    _impl->threadId = 0;
}
} // namespace zeus