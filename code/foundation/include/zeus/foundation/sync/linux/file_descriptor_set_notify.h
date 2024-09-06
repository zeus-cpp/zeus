#pragma once

#ifdef __linux__
#include <memory>
#include <functional>
namespace zeus
{
struct LinuxFileDescriptorSetNotifyImpl;
class LinuxFileDescriptorSetNotify
{
public:
    LinuxFileDescriptorSetNotify(bool automatic = true);
    ~LinuxFileDescriptorSetNotify();
    LinuxFileDescriptorSetNotify(const LinuxFileDescriptorSetNotify&) = delete;
    LinuxFileDescriptorSetNotify(LinuxFileDescriptorSetNotify&& other) noexcept;
    LinuxFileDescriptorSetNotify& operator=(const LinuxFileDescriptorSetNotify&) = delete;
    LinuxFileDescriptorSetNotify& operator=(LinuxFileDescriptorSetNotify&& other) noexcept;
    size_t                        AddFileDescriptorReadableStateCallback(int fileDescriptor, const std::function<void()>& callback);
    size_t                        AddFileDescriptorWritableStateCallback(int fileDescriptor, const std::function<void()>& callback);
    size_t AddFileDescriptorStateCallback(int fileDescriptor, const std::function<void(bool readable, bool writable)>& callback);
    bool   RemoveFileDescriptorStateCallback(size_t callbackId, bool wait = true);
    void   Start();
    void   Stop();
private:
    std::unique_ptr<LinuxFileDescriptorSetNotifyImpl> _impl;
};
} // namespace zeus
#endif
#include "zeus/foundation/core/zeus_compatible.h"
