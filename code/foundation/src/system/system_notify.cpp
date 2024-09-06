#include "zeus/foundation/system/system_notify.h"
#include <cassert>
#include "impl/system_notify_impl.h"

namespace zeus
{
SystemNotify::SystemNotify() : _impl(std::make_unique<SystemNotifyImpl>())
{
}

SystemNotify::SystemNotify(SystemNotify&& other) noexcept : _impl(std::move(other._impl))
{
}

SystemNotify& SystemNotify::operator=(SystemNotify&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

SystemNotify::~SystemNotify()
{
    if (_impl)
    {
        assert(!_impl->run);
    }
}

size_t SystemNotify::AddStorageChangeCallback(const std::function<void(const std::string& path, StorageAction action)>& callback)
{
    return _impl->storageChangeCallbacks.AddCallback(callback);
}

bool SystemNotify::RemoveStorageChangeCallback(size_t id)
{
    return _impl->storageChangeCallbacks.RemoveCallback(id);
}

size_t SystemNotify::AddDeviceChangeCallback(const std::function<void(const std::vector<std::string>& devicePaths, DeviceAction action)>& callback)
{
    return _impl->deviceChangeCallbacks.AddCallback(callback);
}

bool SystemNotify::RemoveDeviceChangeCallback(size_t id)
{
    return _impl->deviceChangeCallbacks.RemoveCallback(id);
}

} // namespace zeus
