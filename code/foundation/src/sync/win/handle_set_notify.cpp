#include "zeus/foundation/sync/win/handle_set_notify.h"
#ifdef _WIN32
#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <cassert>
#include <Windows.h>
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/container/callback_manager.hpp"

namespace zeus
{

struct WinHandleSetNotifyImpl
{
    bool                        automatic = false;
    std::mutex                  mutex;
    std::thread                 thread;
    WinHandle                   event;
    std::atomic<bool>           run = false;
    std::map<size_t, HANDLE>    callbackHandle;
    std::map<HANDLE, size_t>    handleReferenceCount;
    NameCallbackManager<HANDLE> callbackManager;
};

namespace
{
void WaitEvent(WinHandleSetNotifyImpl& impl)
{
    while (impl.run)
    {
        std::vector<HANDLE> handles {impl.event};
        {
            std::unique_lock<std::mutex> lock(impl.mutex);
            handles.reserve(impl.handleReferenceCount.size() + 1);
            for (const auto& [handle, count] : impl.handleReferenceCount)
            {
                assert(count);
                handles.emplace_back(handle);
            }
        }
        assert(!handles.empty());

        const DWORD wait = WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(), FALSE, 1000 * 10);
        if (WAIT_OBJECT_0 == wait)
        {
            continue;
        }

        if (wait > WAIT_OBJECT_0 && wait < WAIT_OBJECT_0 + handles.size())
        {
            assert(WAIT_OBJECT_0 != wait);
            const auto handle = handles[wait - WAIT_OBJECT_0];
            impl.callbackManager.Call(handle);
        }
        if (wait > WAIT_ABANDONED_0 && wait < WAIT_ABANDONED_0 + handles.size())
        {
            assert(WAIT_OBJECT_0 != wait);
            const auto handle = handles[wait - WAIT_OBJECT_0];
            impl.callbackManager.Call(handle);
        }
    }
};
} // namespace

WinHandleSetNotify::WinHandleSetNotify(bool automatic) : _impl(std::make_unique<WinHandleSetNotifyImpl>())
{
    _impl->automatic = automatic;
    _impl->event     = CreateEventW(nullptr, FALSE, FALSE, nullptr);
}

WinHandleSetNotify::~WinHandleSetNotify()
{
    if (_impl)
    {
        if (_impl->automatic)
        {
            Stop();
        }
        assert(!_impl->run);
        assert(!_impl->thread.joinable());
    }
}

WinHandleSetNotify::WinHandleSetNotify(WinHandleSetNotify&& other) noexcept : _impl(std::move(other._impl))
{
}

WinHandleSetNotify& WinHandleSetNotify::operator=(WinHandleSetNotify&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

size_t WinHandleSetNotify::AddHandleStateCallback(HANDLE handle, const std::function<void()>& callback)
{
    size_t id = 0;
    {
        std::unique_lock<std::mutex> lock(_impl->mutex);
        id = _impl->callbackManager.AddCallback(handle, callback);
        _impl->callbackHandle.emplace(id, handle);
        _impl->handleReferenceCount[handle]++;
        SetEvent(_impl->event);
    }
    if (_impl->automatic)
    {
        Start();
    }
    return id;
}

bool WinHandleSetNotify::RemoveHandleStateCallback(size_t callbackId, bool wait)
{
    std::unique_lock<std::mutex> lock(_impl->mutex);
    if (auto handleIter = _impl->callbackHandle.find(callbackId); handleIter != _impl->callbackHandle.end())
    {
        _impl->callbackManager.RemoveCallback(callbackId, wait);
        auto handle        = handleIter->second;
        auto referenceIter = _impl->handleReferenceCount.find(handle);
        assert(referenceIter != _impl->handleReferenceCount.end());
        auto& count = referenceIter->second;
        if (0 == --count)
        {
            _impl->handleReferenceCount.erase(referenceIter);
        }
        _impl->callbackHandle.erase(handleIter);
        SetEvent(_impl->event);
        return true;
    }
    else
    {
        return false;
    }
}

void WinHandleSetNotify::Start()
{
    std::unique_lock<std::mutex> lock(_impl->mutex);
    if (!_impl->run)
    {
        ResetEvent(_impl->event);
        _impl->run    = true;
        _impl->thread = std::thread(std::bind(&WaitEvent, std::ref(*_impl)));
    }
}

void WinHandleSetNotify::Stop()
{
    std::unique_lock<std::mutex> lock(_impl->mutex);
    if (_impl->run)
    {
        _impl->run = false;
        SetEvent(_impl->event);
        if (_impl->thread.joinable())
        {
            _impl->thread.join();
        }
    }
}

} // namespace zeus
#endif