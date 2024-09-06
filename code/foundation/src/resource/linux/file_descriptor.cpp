#ifdef __linux__
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include "zeus/foundation/core/system_error.h"

namespace zeus
{
struct LinuxFileDescriptorImpl
{
    int fd = -1;
};
LinuxFileDescriptor::LinuxFileDescriptor() : _impl(std::make_unique<LinuxFileDescriptorImpl>())
{
}
LinuxFileDescriptor::LinuxFileDescriptor(int fd) : _impl(std::make_unique<LinuxFileDescriptorImpl>())
{
    _impl->fd = fd;
}

LinuxFileDescriptor::LinuxFileDescriptor(LinuxFileDescriptor&& other) noexcept : _impl(std::make_unique<LinuxFileDescriptorImpl>())
{
    _impl.swap(other._impl);
}

LinuxFileDescriptor::~LinuxFileDescriptor()
{
    Close();
}

LinuxFileDescriptor& LinuxFileDescriptor::operator=(LinuxFileDescriptor&& other) noexcept
{
    if (this != &other)
    {
        Close();
        _impl.swap(other._impl);
    }
    return *this;
}

LinuxFileDescriptor& LinuxFileDescriptor::operator=(const int& fd) noexcept
{
    if (!Empty())
    {
        close(_impl->fd);
    }
    _impl->fd = fd;
    return *this;
}

LinuxFileDescriptor::operator int() const noexcept
{
    return _impl->fd;
}

LinuxFileDescriptor::operator int*() const noexcept
{
    return &_impl->fd;
}

LinuxFileDescriptor::operator bool() const noexcept
{
    return !Empty();
}

bool LinuxFileDescriptor::operator==(const LinuxFileDescriptor& other) const noexcept
{
    return _impl->fd == other._impl->fd;
}

bool LinuxFileDescriptor::operator!=(const LinuxFileDescriptor& other) const noexcept
{
    return _impl->fd != other._impl->fd;
}

bool LinuxFileDescriptor::operator==(int fd) const noexcept
{
    return _impl->fd == fd;
}

bool LinuxFileDescriptor::operator!=(int fd) const noexcept
{
    return _impl->fd != fd;
}

void LinuxFileDescriptor::Close()
{
    if (!Empty())
    {
        close(_impl->fd);
        _impl->fd = -1;
    }
}

int LinuxFileDescriptor::Fd() const noexcept
{
    return _impl->fd;
}

int LinuxFileDescriptor::FileDescriptor() const noexcept
{
    return _impl->fd;
}

int LinuxFileDescriptor::NativeHandle() const noexcept
{
    return _impl->fd;
}

bool LinuxFileDescriptor::Empty() const noexcept
{
    return _impl->fd == -1;
}

void LinuxFileDescriptor::Attach(int fd) noexcept
{
    if (!Empty())
    {
        close(_impl->fd);
    }
    _impl->fd = fd;
}

int LinuxFileDescriptor::Detach() noexcept
{
    int fd    = _impl->fd;
    _impl->fd = -1;
    return fd;
}

zeus::expected<LinuxFileDescriptor, std::error_code> LinuxFileDescriptor::Duplicate(bool closeOnExec) const
{
    assert(!Empty());
    int newFd = fcntl(_impl->fd, closeOnExec ? F_DUPFD_CLOEXEC : F_DUPFD, 3);
    if (-1 == newFd)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return LinuxFileDescriptor(newFd);
}

zeus::expected<bool, std::error_code> LinuxFileDescriptor::IsCloseOnExec() const
{
    assert(!Empty());
    int flag = fcntl(_impl->fd, F_GETFD);
    if (-1 == flag)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return flag & FD_CLOEXEC;
}

zeus::expected<void, std::error_code> zeus::LinuxFileDescriptor::SetCloseOnExec(bool closeOnExec)
{
    assert(!Empty());
    if (-1 == fcntl(_impl->fd, F_SETFD, closeOnExec ? FD_CLOEXEC : 0))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

} // namespace zeus
#endif
