#pragma once
#ifdef _WIN32
#include <memory>
#include <zeus/foundation/core/win/win_windef.h>
#include <winsvc.h>
namespace zeus
{
struct WinServiceHandleImpl;
class WinServiceHandle
{
public:
    WinServiceHandle();
    WinServiceHandle(SC_HANDLE handle);
    ~WinServiceHandle();
    WinServiceHandle(const WinServiceHandle&) = delete;
    WinServiceHandle(WinServiceHandle&& other) noexcept;
    WinServiceHandle& operator=(const WinServiceHandle& other) = delete;
    WinServiceHandle& operator=(WinServiceHandle&& other) noexcept;
    WinServiceHandle& operator=(const SC_HANDLE& handle) noexcept;
    operator SC_HANDLE() const noexcept;
    operator SC_HANDLE*() const noexcept;
    explicit operator bool() const noexcept;
    void      Close() noexcept;
    SC_HANDLE Handle() const noexcept;
    bool      Empty() const noexcept;
private:
    std::unique_ptr<WinServiceHandleImpl> _impl;
};
} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
