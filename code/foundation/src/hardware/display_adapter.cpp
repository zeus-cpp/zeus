#include "zeus/foundation/hardware/display_adapter.h"
#include "impl/display_adapter_impl.h"

namespace zeus::Hardware
{

DisplayAdapter::DisplayAdapter() : _impl(std::make_unique<DisplayAdapterImpl>())
{
}
Hardware::DisplayAdapter::~DisplayAdapter()
{
}
Hardware::DisplayAdapter::DisplayAdapter(const DisplayAdapter& other) noexcept : _impl(std::make_unique<DisplayAdapterImpl>(*other._impl))
{
}
Hardware::DisplayAdapter::DisplayAdapter(DisplayAdapter&& other) noexcept : _impl(std::move(other._impl))
{
}
DisplayAdapter& Hardware::DisplayAdapter::operator=(const DisplayAdapter& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
DisplayAdapter& Hardware::DisplayAdapter::operator=(DisplayAdapter&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
uint64_t Hardware::DisplayAdapter::SystemMemory() const noexcept
{
    return _impl->systemMemory;
}
uint64_t Hardware::DisplayAdapter::VideoMemory() const noexcept
{
    return _impl->videoMemory;
}
uint64_t Hardware::DisplayAdapter::SharedMemory() const noexcept
{
    return _impl->sharedMemory;
}
std::string Hardware::DisplayAdapter::DescribeName() const
{
    return _impl->describeName;
}
uint16_t Hardware::DisplayAdapter::VendorId() const noexcept
{
    return _impl->vendorId;
}
std::string Hardware::DisplayAdapter::Vendor() const noexcept
{
    if (_impl->vendorId == 0x10de)
    {
        return "NVIDIA Corporation";
    }
    else if (_impl->vendorId == 0x1002)
    {
        return "Advanced Micro Devices, Inc.";
    }
    else if (_impl->vendorId == 0x8086)
    {
        return "Intel Corporation";
    }
    else if (_impl->vendorId == 0x15ad)
    {
        return "VMware";
    }
    else
    {
        return "Unknown";
    }
}
} // namespace zeus::Hardware
