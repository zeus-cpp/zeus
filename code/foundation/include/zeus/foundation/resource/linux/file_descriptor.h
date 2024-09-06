#pragma once
#ifdef __linux__
#include <memory>
#include <system_error>
#include <sys/types.h>
#include "zeus/expected.hpp"

namespace zeus
{
struct LinuxFileDescriptorImpl;
class LinuxFileDescriptor
{
public:
    LinuxFileDescriptor();
    LinuxFileDescriptor(int fd);
    LinuxFileDescriptor(LinuxFileDescriptor&&) noexcept;
    ~LinuxFileDescriptor();
    LinuxFileDescriptor(const LinuxFileDescriptor&)                                            = delete;
    LinuxFileDescriptor&                                 operator=(const LinuxFileDescriptor&) = delete;
    LinuxFileDescriptor&                                 operator=(LinuxFileDescriptor&&) noexcept;
    LinuxFileDescriptor&                                 operator=(const int& fd) noexcept;
    explicit                                             operator int() const noexcept;
    explicit                                             operator int*() const noexcept;
    explicit                                             operator bool() const noexcept;
    bool                                                 operator==(const LinuxFileDescriptor& other) const noexcept;
    bool                                                 operator!=(const LinuxFileDescriptor& other) const noexcept;
    bool                                                 operator==(int fd) const noexcept;
    bool                                                 operator!=(int fd) const noexcept;
    void                                                 Close();
    int                                                  Fd() const noexcept;
    int                                                  FileDescriptor() const noexcept;
    int                                                  NativeHandle() const noexcept;
    bool                                                 Empty() const noexcept;
    void                                                 Attach(int fd) noexcept;
    int                                                  Detach() noexcept;
    zeus::expected<LinuxFileDescriptor, std::error_code> Duplicate(bool closeOnExec = true) const;
    zeus::expected<bool, std::error_code>                IsCloseOnExec() const;
    zeus::expected<void, std::error_code>                SetCloseOnExec(bool closeOnExec);
private:
    std::unique_ptr<LinuxFileDescriptorImpl> _impl;
};
} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
