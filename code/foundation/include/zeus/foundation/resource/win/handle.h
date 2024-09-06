#pragma once
#ifdef _WIN32
#include <memory>
#include <optional>
#include <system_error>
#include "zeus/expected.hpp"
#include "zeus/foundation/core/win/win_windef.h"

namespace zeus
{
struct WinHandleImpl;
class WinHandle
{
public:
    WinHandle();
    WinHandle(HANDLE handle);
    WinHandle(WinHandle&&) noexcept;
    WinHandle(const WinHandle&) = delete;
    ~WinHandle();
    WinHandle& operator=(const WinHandle&) = delete;
    WinHandle& operator=(WinHandle&& other) noexcept;
    WinHandle& operator=(const HANDLE& handle) noexcept;
    operator HANDLE() const noexcept;
    HANDLE*                                    operator&() noexcept;
    explicit                                   operator bool() const noexcept;
    bool                                       operator==(const WinHandle& other) const noexcept;
    bool                                       operator!=(const WinHandle& other) const noexcept;
    bool                                       operator==(HANDLE handle) const noexcept;
    bool                                       operator!=(HANDLE handle) const noexcept;
    void                                       Close() noexcept;
    HANDLE                                     Handle() const noexcept;
    HANDLE                                     NativeHandle() const noexcept;
    bool                                       Empty() const noexcept;
    void                                       Attach(HANDLE handle) noexcept;
    HANDLE                                     Detach() noexcept;
    zeus::expected<WinHandle, std::error_code> Duplicate(bool inherite = false, const std::optional<DWORD>& desiredAccess = std::nullopt) const;
    zeus::expected<void, std::error_code>      SetInherite(bool inherite);
    zeus::expected<bool, std::error_code>      IsInherited() const;
private:
    std::unique_ptr<WinHandleImpl> _impl;
};
} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
