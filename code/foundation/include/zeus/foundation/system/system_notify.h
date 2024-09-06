#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace zeus
{
struct SystemNotifyImpl;
class SystemNotify
{
public:
    enum class StorageAction
    {
        Unknown,
        Mount,
        Unmount
    };
    enum class DeviceAction
    {
        Unknown,
        Plug,
        Unplug
    };
    SystemNotify();
    ~SystemNotify();
    SystemNotify(const SystemNotify&) = delete;
    SystemNotify(SystemNotify&& other) noexcept;
    SystemNotify& operator=(const SystemNotify&) = delete;
    SystemNotify& operator=(SystemNotify&& other) noexcept;

    bool   Start();
    void   Stop();
    size_t AddStorageChangeCallback(const std::function<void(const std::string& path, StorageAction action)>& callback);
    bool   RemoveStorageChangeCallback(size_t id);
    size_t AddDeviceChangeCallback(const std::function<void(const std::vector<std::string>& devicePaths, DeviceAction action)>& callback);
    bool   RemoveDeviceChangeCallback(size_t id);

private:
    std::unique_ptr<SystemNotifyImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
