#include "zeus/foundation/sync/file_mutex.h"
#ifdef _WIN32
#include <thread>
#include <cassert>
#include <filesystem>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/win/handle.h"

namespace fs = std::filesystem;

namespace zeus
{

struct FileMutexImpl
{
    fs::path  path;
    WinHandle handle;
#ifndef _NDEBUG
    bool isOwner = false;
#endif
};
FileMutex::FileMutex() : _impl(std::make_unique<FileMutexImpl>())
{
}

FileMutex::FileMutex(FileMutex&& other) noexcept : _impl(std::move(other._impl))
{
}

FileMutex& FileMutex::operator=(FileMutex&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

std::filesystem::path FileMutex::Path() const
{
    return _impl->path;
}

FileMutex::~FileMutex()
{
#ifndef _NDEBUG
    if (_impl)
    {
        assert(!_impl->isOwner);
    }
#endif
}

zeus::expected<void, std::error_code> FileMutex::LockExclusive()
{
    assert(_impl->handle);
    OVERLAPPED overlapped = {};
    if (LockFileEx(_impl->handle, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlapped))
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> zeus::FileMutex::LockShared()
{
    assert(_impl->handle);
    OVERLAPPED overlapped = {};
    if (LockFileEx(_impl->handle, 0, 0, 1, 0, &overlapped))
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> FileMutex::Unlock()
{
    assert(_impl->handle);
    OVERLAPPED overlapped = {};
#ifndef _NDEBUG
    assert(_impl->isOwner);
    _impl->isOwner = false;
#endif
    if (UnlockFileEx(_impl->handle, 0, 1, 0, &overlapped))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<bool, std::error_code> FileMutex::TryLockExclusive()
{
    assert(_impl->handle);
    OVERLAPPED overlapped = {};
    if (LockFileEx(_impl->handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &overlapped))
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return true;
    }
    else
    {
        const auto error = GetLastError();
        if (ERROR_LOCK_VIOLATION == error)
        {
            return false;
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
}

zeus::expected<bool, std::error_code> zeus::FileMutex::TryLockShared()
{
    assert(_impl->handle);
    OVERLAPPED overlapped = {};
    if (LockFileEx(_impl->handle, LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &overlapped))
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return true;
    }
    else
    {
        const auto error = GetLastError();
        if (ERROR_LOCK_VIOLATION == error)
        {
            return false;
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
}

zeus::expected<FileMutex, std::error_code> FileMutex::Create(const std::filesystem::path& filePath)
{
    SECURITY_ATTRIBUTES securittyAttr = {};
    securittyAttr.nLength             = sizeof(SECURITY_ATTRIBUTES);
    securittyAttr.bInheritHandle      = FALSE;
    SECURITY_DESCRIPTOR sd            = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    SetSecurityDescriptorGroup(&sd, nullptr, FALSE);
    SetSecurityDescriptorSacl(&sd, FALSE, nullptr, FALSE);
    securittyAttr.lpSecurityDescriptor = &sd;
    WinHandle handle = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &securittyAttr, OPEN_EXISTING, 0, nullptr);
    if (handle)
    {
        FileMutex mutex;
        mutex._impl->handle = std::move(handle);
        mutex._impl->path   = filePath;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
} // namespace zeus

#endif
