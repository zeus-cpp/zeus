#include "zeus/foundation/hardware/hard_disk.h"
#include "impl/hard_disk_impl.h"
namespace zeus::Hardware
{
HardDisk::HardDisk() : _impl(std::make_unique<HardDiskImpl>())
{
}
HardDisk::~HardDisk()
{
}
HardDisk::HardDisk(const HardDisk& other) : _impl(std::make_unique<HardDiskImpl>(*other._impl))
{
}
HardDisk::HardDisk(HardDisk&& other) noexcept : _impl(std::move(other._impl))
{
}
HardDisk& HardDisk::operator=(const HardDisk& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
HardDisk& HardDisk::operator=(HardDisk&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string HardDisk::Model() const
{
    return _impl->model;
}
std::string HardDisk::SerialNumber() const
{
    return _impl->serialNumber;
}
uint64_t HardDisk::Capacity() const
{
    return _impl->capacity;
}
bool HardDisk::IsSSD() const
{
    return _impl->isSSD;
}
bool HardDisk::IsRemovable() const
{
    return _impl->isRemovable;
}
std::string HardDisk::Location() const
{
    return _impl->location;
}
HardDisk::HardDiskBusType HardDisk::BusType() const
{
    return _impl->busType;
}

std::string HardDisk::BusTypeName(HardDiskBusType type)
{
    switch (type)
    {
    case HardDiskBusType::kUSB:
        return "USB";
    case HardDiskBusType::kATA:
        return "ATA";
    case HardDiskBusType::kSD:
        return "SD";
    case HardDiskBusType::kSAS:
        return "SAS";
    case HardDiskBusType::kSATA:
        return "SATA";
    case HardDiskBusType::kSCSI:
        return "SCSI";
    case HardDiskBusType::kNVMe:
        return "NVMe";
    case HardDiskBusType::kRAID:
        return "RAID";
    case HardDiskBusType::kVirtual:
        return "Virtual";
    default:
        return "Unknown";
    }
}
} // namespace zeus::Hardware