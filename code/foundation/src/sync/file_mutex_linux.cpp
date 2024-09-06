#include "zeus/foundation/sync/file_mutex.h"
#ifdef __linux__
#include <thread>
#include <memory>
#include <shared_mutex>
#include <filesystem>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"

namespace fs = std::filesystem;

namespace zeus
{
struct FileMutexImpl
{
    fs::path            path;
    LinuxFileDescriptor file;
    std::shared_mutex   mutex;
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
    assert(!_impl->file.Empty());
    _impl->mutex.lock();
    if (0 == flock(_impl->file.Fd(), LOCK_EX))
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

zeus::expected<void, std::error_code> FileMutex::LockShared()
{
    assert(!_impl->file.Empty());
    _impl->mutex.lock_shared();
    if (0 == flock(_impl->file.Fd(), LOCK_SH))
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

zeus::expected<void, std::error_code> FileMutex::Unlock()
{
    assert(!_impl->file.Empty());
#ifndef _NDEBUG
    assert(_impl->isOwner);
    _impl->isOwner = false;
#endif
    if (0 == flock(_impl->file.Fd(), LOCK_UN))
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

zeus::expected<bool, std::error_code> FileMutex::TryLockExclusive()
{
    assert(!_impl->file.Empty());
    if (!_impl->mutex.try_lock())
    {
        return false;
    }
    if (0 == flock(_impl->file.Fd(), LOCK_EX | LOCK_NB))
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return true;
    }
    else
    {
        _impl->mutex.unlock();
        if (EWOULDBLOCK == errno)
        {
            return false;
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
}

zeus::expected<bool, std::error_code> FileMutex::TryLockShared()
{
    assert(!_impl->file.Empty());
    if (!_impl->mutex.try_lock_shared())
    {
        return false;
    }
    if (0 == flock(_impl->file.Fd(), LOCK_SH | LOCK_NB))
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return true;
    }
    else
    {
        _impl->mutex.unlock();
        if (EWOULDBLOCK == errno)
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
    LinuxFileDescriptor file(open(filePath.c_str(), O_CLOEXEC | O_RDWR));
    if (!file.Empty())
    {
        FileMutex mutex;
        mutex._impl->file = std::move(file);
        mutex._impl->path = filePath;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
} // namespace zeus
#endif
