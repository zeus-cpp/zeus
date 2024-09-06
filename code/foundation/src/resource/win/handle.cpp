#ifdef _WIN32
#include "zeus/foundation/resource/win/handle.h"
#include <utility>
#include <cassert>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"

namespace zeus
{
struct WinHandleImpl
{
    HANDLE handle = nullptr;
};
WinHandle::WinHandle() : _impl(std::make_unique<WinHandleImpl>())
{
}
WinHandle::WinHandle(HANDLE handle) : _impl(std::make_unique<WinHandleImpl>())
{
    _impl->handle = handle;
}

WinHandle::WinHandle(WinHandle&& other) noexcept : _impl(std::make_unique<WinHandleImpl>())
{
    _impl.swap(other._impl);
}

WinHandle::~WinHandle()
{
    Close();
}

WinHandle& WinHandle::operator=(WinHandle&& other) noexcept
{
    if (this != std::addressof(other))
    {
        Close();
        _impl.swap(other._impl);
    }
    return *this;
}

WinHandle& WinHandle::operator=(const HANDLE& handle) noexcept
{
    Attach(handle);
    return *this;
}

WinHandle::operator HANDLE() const noexcept
{
    return _impl->handle;
}

HANDLE* WinHandle::operator&() noexcept
{
    assert(Empty());
    return &_impl->handle;
}

WinHandle::operator bool() const noexcept
{
    return !Empty();
}

bool WinHandle::operator==(const WinHandle& other) const noexcept
{
    return _impl->handle == other._impl->handle;
}

bool WinHandle::operator!=(const WinHandle& other) const noexcept
{
    return _impl->handle != other._impl->handle;
}

bool WinHandle::operator==(HANDLE handle) const noexcept
{
    return _impl->handle == handle;
}

bool WinHandle::operator!=(HANDLE handle) const noexcept
{
    return _impl->handle != handle;
}

void WinHandle::Close() noexcept
{
    if (!Empty())
    {
        CloseHandle(_impl->handle);
        _impl->handle = nullptr;
    }
}

HANDLE WinHandle::Handle() const noexcept
{
    return _impl->handle;
}

HANDLE WinHandle::NativeHandle() const noexcept
{
    return _impl->handle;
}

bool WinHandle::Empty() const noexcept
{
    return !_impl->handle || _impl->handle == INVALID_HANDLE_VALUE;
}

void WinHandle::Attach(HANDLE handle) noexcept
{
    if (!Empty())
    {
        CloseHandle(_impl->handle);
    }
    _impl->handle = handle;
}

HANDLE WinHandle::Detach() noexcept
{
    HANDLE handle = _impl->handle;
    _impl->handle = nullptr;
    return handle;
}

zeus::expected<WinHandle, std::error_code> WinHandle::Duplicate(bool inherite, const std::optional<DWORD>& desiredAccess) const
{
    assert(!Empty());
    HANDLE target = nullptr;
    if (DuplicateHandle(
            GetCurrentProcess(), _impl->handle, GetCurrentProcess(), &target, desiredAccess.value_or(0), inherite,
            desiredAccess.has_value() ? 0 : DUPLICATE_SAME_ACCESS
        ))
    {
        return WinHandle(target);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinHandle::SetInherite(bool inherite)
{
    assert(!Empty());
    if (SetHandleInformation(_impl->handle, HANDLE_FLAG_INHERIT, inherite ? HANDLE_FLAG_INHERIT : 0))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<bool, std::error_code> WinHandle::IsInherited() const
{
    assert(!Empty());
    DWORD flag = 0;
    if (GetHandleInformation(_impl->handle, &flag))
    {
        return flag & HANDLE_FLAG_INHERIT;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

} // namespace zeus
#endif
