#include "zeus/foundation/hardware/hard_disk.h"
#ifdef __linux__
#include <cstring>
#include <filesystem>
#include <libudev.h>
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "impl/hard_disk_impl.h"

namespace fs = std::filesystem;

namespace zeus::Hardware
{
namespace
{
uint64_t ParseDiskSize(const fs::path& diskDir)
{
    auto sizeString = Trim(FileContent(diskDir / "size").value_or(""));
    if (!sizeString.empty() && !StartWith(sizeString, "-"))
    {
        try
        {
            // 512 bytes per sector
            return std::stoll(sizeString) / 2;
        }
        catch (...)
        {
        }
    }
    return 0;
}

bool ParseDiskIsRemovable(const fs::path& diskDir)
{
    auto removableString = Trim(FileContent(diskDir / "removable").value_or(""));
    return "1" == removableString;
}

bool ParseDiskIsSSD(const fs::path& diskDir)
{
    auto rotationalString = Trim(FileContent(diskDir / "queue" / "rotational").value_or(""));
    return "0" == rotationalString;
}

void ParseDiskModelAndSerialAndBusType(
    const std::string& deviceName, std::string& model, std::string& serialNumber, HardDisk::HardDiskBusType& busType
)
{
    udev* udev = udev_new();
    if (!udev)
    {
        return;
    }
    AutoRelease udevRelease(
        [&udev]()
        {
            if (udev)
            {
                udev_unref(udev);
            }
        }
    );
    udev_device* dev = udev_device_new_from_subsystem_sysname(udev, "block", deviceName.c_str());
    if (!dev)
    {
        return;
    }
    AutoRelease devRelease(
        [&dev]()
        {
            if (dev)
            {
                udev_device_unref(dev);
            }
        }
    );
    const char* data = nullptr;
    data             = udev_device_get_property_value(dev, "ID_MODEL");
    if (data)
    {
        model = data;
    }
    data = udev_device_get_property_value(dev, "ID_SERIAL_SHORT");
    if (data)
    {
        serialNumber = data;
    }
    if (StartWith(deviceName, "nvme"))
    {
        busType = HardDisk::HardDiskBusType::kNVMe;
        return;
    }
    data = udev_device_get_property_value(dev, "ID_BUS");
    if (!data)
    {
        busType = HardDisk::HardDiskBusType::kVirtual;
    }
    else if (0 == strcmp(data, "ata"))
    {
        data = udev_device_get_property_value(dev, "ID_ATA_SATA");
        if (0 == strcmp(data, "1"))
        {
            busType = HardDisk::HardDiskBusType::kSATA;
        }
        else
        {
            busType = HardDisk::HardDiskBusType::kATA;
        }
    }
    else if (0 == strcmp(data, "usb"))
    {
        busType = HardDisk::HardDiskBusType::kUSB;
    }
    else if (0 == strcmp(data, "scsi"))
    {
        busType = HardDisk::HardDiskBusType::kSCSI;
    }
    else
    {
        busType = HardDisk::HardDiskBusType::kUnknown;
    }
}

} // namespace
std::vector<HardDisk> HardDisk::ListAll()
{
    std::vector<HardDisk> disks;
    std::error_code       ec;
    for (auto& item : fs::directory_iterator("/sys/block/", ec))
    {
        const auto& diskDir    = item.path();
        auto        deviceName = diskDir.filename().string();
        if (!StartWith(deviceName, "sd") && !StartWith(deviceName, "nvme") && !StartWith(deviceName, "hd") && !StartWith(deviceName, "md"))
        {
            continue;
        }
        HardDisk disk;
        disk._impl->capacity    = ParseDiskSize(diskDir);
        disk._impl->location    = deviceName;
        disk._impl->isSSD       = ParseDiskIsSSD(diskDir);
        disk._impl->isRemovable = ParseDiskIsRemovable(diskDir);
        ParseDiskModelAndSerialAndBusType(deviceName, disk._impl->model, disk._impl->serialNumber, disk._impl->busType);
        disks.emplace_back(std::move(disk));
    }
    return disks;
}
std::vector<HardDisk> HardDisk::ListRemovable(bool removable)
{
    std::vector<HardDisk> disks;
    std::error_code       ec;
    for (auto& item : fs::directory_iterator("/sys/block/", ec))
    {
        const auto& diskDir    = item.path();
        auto        deviceName = diskDir.filename().string();
        if (!StartWith(deviceName, "sd") && !StartWith(deviceName, "nvme") && !StartWith(deviceName, "hd") && !StartWith(deviceName, "md"))
        {
            continue;
        }
        if (removable != ParseDiskIsRemovable(diskDir))
        {
            continue;
        }
        HardDisk disk;
        disk._impl->capacity    = ParseDiskSize(diskDir);
        disk._impl->location    = deviceName;
        disk._impl->isSSD       = ParseDiskIsSSD(diskDir);
        disk._impl->isRemovable = removable;
        ParseDiskModelAndSerialAndBusType(deviceName, disk._impl->model, disk._impl->serialNumber, disk._impl->busType);
        disks.emplace_back(std::move(disk));
    }
    return disks;
}
} // namespace zeus::Hardware
#endif
