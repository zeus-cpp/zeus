#include "zeus/foundation/resource/win/service_handle.h"

#ifdef _WIN32
namespace zeus
{
struct WinServiceHandleImpl
{
    SC_HANDLE handle = nullptr;
};

WinServiceHandle::WinServiceHandle() : _impl(std::make_unique<WinServiceHandleImpl>())
{
}
WinServiceHandle::WinServiceHandle(SC_HANDLE handle) : _impl(std::make_unique<WinServiceHandleImpl>())
{
    _impl->handle = handle;
}

WinServiceHandle::~WinServiceHandle()
{
    if (!Empty())
    {
        CloseServiceHandle(_impl->handle);
    }
}

WinServiceHandle::WinServiceHandle(WinServiceHandle&& other) noexcept : _impl(std::make_unique<WinServiceHandleImpl>())
{
    _impl.swap(other._impl);
}

WinServiceHandle& WinServiceHandle::operator=(WinServiceHandle&& other) noexcept
{
    if (this != std::addressof(other))
    {
        Close();
        _impl.swap(other._impl);
    }
    return *this;
}

WinServiceHandle& WinServiceHandle::operator=(const SC_HANDLE& handle) noexcept
{
    _impl->handle = handle;
    return *this;
}

WinServiceHandle::operator SC_HANDLE() const noexcept
{
    return _impl->handle;
}

WinServiceHandle::operator SC_HANDLE*() const noexcept
{
    return &_impl->handle;
}

WinServiceHandle::operator bool() const noexcept
{
    return !Empty();
}

void WinServiceHandle::Close() noexcept
{
    if (!Empty())
    {
        CloseServiceHandle(_impl->handle);
        _impl->handle = nullptr;
    }
}

SC_HANDLE WinServiceHandle::Handle() const noexcept
{
    return _impl->handle;
}
bool WinServiceHandle::Empty() const noexcept
{
    return !_impl->handle;
}

} // namespace zeus
#endif
