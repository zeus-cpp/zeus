#include "zeus/foundation/hardware/hard_disk.h"
#ifdef _WIN32
#include <sstream>
#include <functional>
#include <Windows.h>
#include <devguid.h>
#include <SetupAPI.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/resource/win/handle.h"
#include "impl/hard_disk_impl.h"

#pragma comment(lib, "Setupapi.lib")

namespace zeus::Hardware
{

namespace
{
void ForeachDiskSetup(const std::function<void(const WinHandle& file, const std::string& model)>& handler)
{
    auto            devClass = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_DISK, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    AutoRelease     devRelease([&devClass]() { SetupDiDestroyDeviceInfoList(devClass); });
    SP_DEVINFO_DATA devInfo;
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    DWORD index    = 0;
    while (SetupDiEnumDeviceInfo(devClass, index, &devInfo))
    {
        std::string diskModel;
        DWORD       requiredSize = 0;
        SetupDiGetDeviceRegistryPropertyW(devClass, &devInfo, SPDRP_FRIENDLYNAME, nullptr, nullptr, 0, &requiredSize);
        const auto error = GetLastError();
        if (error == ERROR_INSUFFICIENT_BUFFER)
        {
            std::unique_ptr<wchar_t[]> name = std::make_unique<wchar_t[]>(requiredSize / 2);
            if (SetupDiGetDeviceRegistryPropertyW(
                    devClass, &devInfo, SPDRP_FRIENDLYNAME, nullptr, reinterpret_cast<PBYTE>(name.get()), requiredSize, &requiredSize
                ))
            {
                diskModel = CharsetUtils::UnicodeToUTF8(name.get());
            }
        }

        SP_DEVICE_INTERFACE_DATA devInterfaceData;
        devInterfaceData.cbSize = sizeof(devInterfaceData);
        SetupDiEnumDeviceInterfaces(devClass, &devInfo, &GUID_DEVINTERFACE_DISK, 0, &devInterfaceData);
        SetupDiGetDeviceInterfaceDetailW(devClass, &devInterfaceData, nullptr, 0, &requiredSize, &devInfo);
        auto  buffer      = std::make_unique<uint8_t[]>(requiredSize);
        auto* detalData   = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(buffer.get());
        detalData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        //获取内核路径
        if (SetupDiGetDeviceInterfaceDetailW(devClass, &devInterfaceData, detalData, requiredSize, nullptr, &devInfo))
        {
            WinHandle file =
                CreateFileW(detalData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (!file.Empty())
            {
                handler(file, diskModel);
            }
        }
        ++index;
    }
}

//KB
uint64_t GetHardDiskCapacity(const WinHandle& file)
{
    static constexpr uint32_t KB          = 1024;
    DWORD                     bytesReturn = 0;
    DISK_GEOMETRY             geometry;
    if (::DeviceIoControl(file, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &geometry, sizeof(geometry), &bytesReturn, nullptr))
    {
        return geometry.Cylinders.QuadPart * geometry.TracksPerCylinder * geometry.SectorsPerTrack * geometry.BytesPerSector / KB;
    }
    return 0;
}

bool HardDiskIsSSD(const WinHandle& file)
{
    STORAGE_PROPERTY_QUERY query       = {};
    DWORD                  bytesReturn = 0;
    query.PropertyId                   = StorageDeviceTrimProperty;
    query.QueryType                    = PropertyStandardQuery;

    DEVICE_TRIM_DESCRIPTOR dtd = {};
    if (::DeviceIoControl(file, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &dtd, sizeof(dtd), &bytesReturn, nullptr) &&
        bytesReturn == sizeof(dtd))
    {
        return dtd.TrimEnabled;
    }
    return false;
}

void GetHardDiskLocation(const WinHandle& file, std::string& location)
{
    SCSI_ADDRESS SCSIAddress    = {};
    ULONG        returnedLength = 0;

    if (DeviceIoControl(file, IOCTL_SCSI_GET_ADDRESS, nullptr, 0, &SCSIAddress, sizeof(SCSIAddress), &returnedLength, nullptr))
    {
        std::stringstream stream;
        stream << static_cast<size_t>(SCSIAddress.PathId) << "-" << static_cast<size_t>(SCSIAddress.TargetId) << "-"
               << static_cast<size_t>(SCSIAddress.Lun) << "-" << static_cast<size_t>(SCSIAddress.PortNumber) << "-"
               << static_cast<size_t>(SCSIAddress.Length);
        location = stream.str();
    }
}

void GetHardDiskProperty(const WinHandle& file, HardDisk::HardDiskBusType& busType, std::string& serialNumber, bool& removable)
{
    STORAGE_PROPERTY_QUERY query       = {};
    DWORD                  bytesReturn = 0;
    query.PropertyId                   = StorageDeviceProperty;
    query.QueryType                    = PropertyStandardQuery;
    STORAGE_DESCRIPTOR_HEADER header;
    if (::DeviceIoControl(file, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &header, sizeof(header), &bytesReturn, nullptr) &&
        bytesReturn == sizeof(header))
    {
        auto buffer = std::make_unique<uint8_t[]>(header.Size);
        if (::DeviceIoControl(file, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer.get(), header.Size, &bytesReturn, nullptr) &&
            bytesReturn == header.Size)
        {
            const auto* descriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer.get());
            switch (descriptor->BusType)
            {
            case STORAGE_BUS_TYPE::BusTypeUsb:
                busType = HardDisk::HardDiskBusType::kUSB;
                break;
            case STORAGE_BUS_TYPE::BusTypeAta:
                busType = HardDisk::HardDiskBusType::kATA;
                break;
            case STORAGE_BUS_TYPE::BusTypeSd:
                busType = HardDisk::HardDiskBusType::kSD;
                break;
            case STORAGE_BUS_TYPE::BusTypeSas:
                busType = HardDisk::HardDiskBusType::kSAS;
                break;
            case STORAGE_BUS_TYPE::BusTypeSata:
                busType = HardDisk::HardDiskBusType::kSATA;
                break;
            case STORAGE_BUS_TYPE::BusTypeScsi:
                busType = HardDisk::HardDiskBusType::kSCSI;
                break;
            case STORAGE_BUS_TYPE::BusTypeNvme:
                busType = HardDisk::HardDiskBusType::kNVMe;
                break;
            case STORAGE_BUS_TYPE::BusTypeRAID:
                busType = HardDisk::HardDiskBusType::kRAID;
                break;
            case STORAGE_BUS_TYPE::BusTypeFileBackedVirtual:
            case STORAGE_BUS_TYPE::BusTypeVirtual:
                busType = HardDisk::HardDiskBusType::kVirtual;
                break;
            default:
                busType = HardDisk::HardDiskBusType::kUnknown;
                break;
            }
            if (descriptor->SerialNumberOffset)
            {
                serialNumber = Trim(std::string(reinterpret_cast<char*>(buffer.get() + descriptor->SerialNumberOffset)));
            }
            removable = descriptor->RemovableMedia;
        }
    }
}

} // namespace
std::vector<HardDisk> HardDisk::ListAll()
{
    std::vector<HardDisk> disks;
    ForeachDiskSetup(
        [&disks](const WinHandle& file, const std::string& model)
        {
            HardDisk disk;
            GetHardDiskProperty(file, disk._impl->busType, disk._impl->serialNumber, disk._impl->isRemovable);
            disk._impl->model    = model;
            disk._impl->capacity = GetHardDiskCapacity(file);
            disk._impl->isSSD    = HardDiskIsSSD(file);
            GetHardDiskLocation(file, disk._impl->location);
            disks.emplace_back(std::move(disk));
        }
    );
    return disks;
}
std::vector<HardDisk> HardDisk::ListRemovable(bool removable)
{
    std::vector<HardDisk> disks;
    ForeachDiskSetup(
        [&disks, removable](const WinHandle& file, const std::string& model)
        {
            HardDisk disk;
            GetHardDiskProperty(file, disk._impl->busType, disk._impl->serialNumber, disk._impl->isRemovable);
            if (removable != disk._impl->isRemovable)
            {
                return;
            }
            disk._impl->model    = model;
            disk._impl->capacity = GetHardDiskCapacity(file);
            disk._impl->isSSD    = HardDiskIsSSD(file);
            GetHardDiskLocation(file, disk._impl->location);
            disks.emplace_back(std::move(disk));
        }
    );
    return disks;
}
} // namespace zeus::Hardware
#endif