#pragma once

#ifdef _WIN32
#include <memory>
#include <functional>
#include "zeus/foundation/core/win/win_windef.h"
namespace zeus
{
struct WinHandleSetNotifyImpl;
class WinHandleSetNotify
{
public:
    WinHandleSetNotify(bool automatic = true);
    ~WinHandleSetNotify();
    WinHandleSetNotify(const WinHandleSetNotify&) = delete;
    WinHandleSetNotify(WinHandleSetNotify&& other) noexcept;
    WinHandleSetNotify& operator=(const WinHandleSetNotify&) = delete;
    WinHandleSetNotify& operator=(WinHandleSetNotify&& other) noexcept;
    size_t              AddHandleStateCallback(HANDLE handle, const std::function<void()>& callback);
    bool                RemoveHandleStateCallback(size_t callbackId, bool wait = true);
    void                Start();
    void                Stop();
private:
    std::unique_ptr<WinHandleSetNotifyImpl> _impl;
};
} // namespace zeus
#endif
#include "zeus/foundation/core/zeus_compatible.h"
