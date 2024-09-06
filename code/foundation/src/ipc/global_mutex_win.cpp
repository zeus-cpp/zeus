#include "zeus/foundation/ipc/global_mutex.h"
#ifdef _WIN32
#include <mutex>
#include <cassert>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/string/charset_utils.h"

namespace zeus
{

struct GlobalMutexImpl
{
    std::string name;
    std::mutex  mutex;
    WinHandle   handle;
#ifndef _NDEBUG
    bool isOwner = false;
#endif
};

GlobalMutex::GlobalMutex() : _impl(std::make_unique<GlobalMutexImpl>())
{
}

GlobalMutex::GlobalMutex(GlobalMutex&& other) noexcept : _impl(std::move(other._impl))
{
}

GlobalMutex& GlobalMutex::operator=(GlobalMutex&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

GlobalMutex::~GlobalMutex()
{
#ifndef _NDEBUG
    if (_impl)
    {
        assert(!_impl->isOwner);
    }
#endif
}

std::string GlobalMutex::Name() const
{
    return _impl->name;
}

zeus::expected<void, std::error_code> GlobalMutex::Lock()
{
    assert(_impl->handle);
    _impl->mutex.lock();
    const DWORD result = WaitForSingleObject(_impl->handle, INFINITE);
    if (WAIT_OBJECT_0 == result || WAIT_ABANDONED == result)
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return {};
    }
    else
    {
        _impl->mutex.unlock();
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> GlobalMutex::Unlock()
{
    assert(_impl->handle);
#ifndef _NDEBUG
    assert(_impl->isOwner);
    _impl->isOwner = false;
#endif
    if (ReleaseMutex(_impl->handle))
    {
        _impl->mutex.unlock();
        return {};
    }
    else
    {
        _impl->mutex.unlock();
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<bool, std::error_code> GlobalMutex::TryLock()
{
    assert(_impl->handle);
    if (!_impl->mutex.try_lock())
    {
        return false;
    }
    const DWORD result = WaitForSingleObject(_impl->handle, 0);
    switch (result)
    {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return true;
    case WAIT_TIMEOUT:
        _impl->mutex.unlock();
        return false;
    default:
        _impl->mutex.unlock();
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<GlobalMutex, std::error_code> GlobalMutex::OpenOrCreate(const std::string& name)
{
    auto                wname         = CharsetUtils::UTF8ToUnicode(name);
    SECURITY_ATTRIBUTES securittyAttr = {};
    securittyAttr.nLength             = sizeof(SECURITY_ATTRIBUTES);
    securittyAttr.bInheritHandle      = FALSE;
    SECURITY_DESCRIPTOR sd            = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    SetSecurityDescriptorGroup(&sd, nullptr, FALSE);
    SetSecurityDescriptorSacl(&sd, FALSE, nullptr, FALSE);
    securittyAttr.lpSecurityDescriptor = &sd;
    WinHandle handle                   = CreateMutexW(&securittyAttr, FALSE, wname.empty() ? nullptr : wname.c_str());
    if (handle)
    {
        GlobalMutex mutex;
        mutex._impl->handle = std::move(handle);
        mutex._impl->name   = name;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<GlobalMutex, std::error_code> GlobalMutex::Open(const std::string& name)
{
    auto      wname  = CharsetUtils::UTF8ToUnicode(name);
    WinHandle handle = OpenMutexW(SYNCHRONIZE, FALSE, wname.c_str());
    if (handle)
    {
        GlobalMutex mutex;
        mutex._impl->handle = std::move(handle);
        mutex._impl->name   = name;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
} // namespace zeus
#endif