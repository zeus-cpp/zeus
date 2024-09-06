#include "zeus/foundation/hardware/storage.h"
#ifdef _WIN32
#include <optional>
#include <array>
#include <sstream>
#include <Windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/resource/win/handle.h"
#include "impl/storage_impl.h"
namespace zeus::Hardware
{
namespace
{
//结果只有盘符，没有冒号
std::vector<char> GetAllLogicalDrives()
{
    std::vector<char> result;
    //只遍历26个字母
    DWORD             drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++)
    {
        if (drives & 1)
        {
            result.emplace_back('A' + i);
        }
        drives = drives >> 1;
    }
    return result;
}

std::wstring GetDriveRoot(char drive)
{
    std::wstring root;
    root.push_back(static_cast<wchar_t>(drive));
    root.append(LR"(:\)");
    return root;
}

bool IsFloppyDisk(char drive)
{
    std::wstring path(LR"(\\.\)");
    path.push_back(drive);
    path.push_back(':');
    WinHandle     file = CreateFileW(path.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD         bytesReturn = 0;
    DISK_GEOMETRY geometry;
    if (::DeviceIoControl(file, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &geometry, sizeof(geometry), &bytesReturn, nullptr))
    {
        if (geometry.MediaType != MEDIA_TYPE::FixedMedia && geometry.MediaType != MEDIA_TYPE::RemovableMedia)
        {
            return true;
        }
    }
    return false;
}

std::optional<Storage::StorageType> ParseDriveType(char drive)
{
    auto root = GetDriveRoot(drive);
    switch (GetDriveTypeW(root.c_str()))
    {
    case DRIVE_REMOVABLE:
        if (IsFloppyDisk(drive))
        {
            return Storage::StorageType::kFloppy;
        }
        else
        {
            return Storage::StorageType::kDisk;
        }
    case DRIVE_FIXED:
        return Storage::StorageType::kDisk;
    case DRIVE_REMOTE:
        return Storage::StorageType::kRemote;
    case DRIVE_CDROM:
        return Storage::StorageType::kCD;
    case DRIVE_RAMDISK:
        return Storage::StorageType::kRAM;
    default:
        return std::nullopt;

    } // namespace
}

void ParseStorageName(char drive, std::string& name, std::string& fileSystem)
{
    auto                              root           = GetDriveRoot(drive);
    std::array<wchar_t, MAX_PATH + 1> volumeName     = {0};
    std::array<wchar_t, MAX_PATH + 1> filesystemName = {0};
    DWORD                             maxPathLength  = 0;
    DWORD                             systemFlag     = 0;
    if (GetVolumeInformationW(
            root.data(), volumeName.data(), MAX_PATH + 1, nullptr, &maxPathLength, &systemFlag, filesystemName.data(), MAX_PATH + 1
        ))
    {
        name       = zeus::CharsetUtils::UnicodeToUTF8(volumeName.data());
        fileSystem = zeus::CharsetUtils::UnicodeToUTF8(filesystemName.data());
    }
}

void GetStorageSpace(char drive, uint64_t& totalCapacity, uint64_t& freeCapacity, uint64_t& availableCapacity)
{
    auto           root = GetDriveRoot(drive);
    ULARGE_INTEGER total;
    ULARGE_INTEGER free;
    if (GetDiskFreeSpaceExW(root.c_str(), &free, &total, nullptr))
    {
        totalCapacity     = total.QuadPart / 1024;
        freeCapacity      = free.QuadPart / 1024;
        availableCapacity = totalCapacity;
    }
}

void GetStorageLocation(wchar_t drive, std::string& location)
{
    std::wstring path(LR"(\\.\)");
    path.push_back(drive);
    path.push_back(':');
    WinHandle    file = CreateFileW(path.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    SCSI_ADDRESS SCSIAddress = {};
    ULONG        returnedLength;

    if (DeviceIoControl(file, IOCTL_SCSI_GET_ADDRESS, nullptr, 0, &SCSIAddress, sizeof(SCSIAddress), &returnedLength, nullptr))
    {
        std::stringstream stream;
        stream << static_cast<size_t>(SCSIAddress.PathId) << "-" << static_cast<size_t>(SCSIAddress.TargetId) << "-"
               << static_cast<size_t>(SCSIAddress.Lun) << "-" << static_cast<size_t>(SCSIAddress.PortNumber) << "-" << SCSIAddress.Length;
        location = stream.str();
    }
}

} // namespace
std::vector<Storage> Storage::ListAll()
{
    std::vector<Storage> storages;
    auto                 drives = GetAllLogicalDrives();
    DWORD                oldErr = 0;
    SetThreadErrorMode(SEM_FAILCRITICALERRORS, &oldErr);
    for (const auto& drive : drives)
    {
        auto storageType = ParseDriveType(drive);
        if (!storageType.has_value())
        {
            continue;
        }

        Storage storage;
        storage._impl->path.push_back(drive);
        storage._impl->path.push_back(':');
        storage._impl->type = *storageType;
        ParseStorageName(drive, storage._impl->name, storage._impl->fileSystem);
        GetStorageSpace(drive, storage._impl->totalCapacity, storage._impl->freeCapacity, storage._impl->availableCapacity);
        GetStorageLocation(drive, storage._impl->location);
        storages.emplace_back(std::move(storage));
    }
    SetThreadErrorMode(oldErr, nullptr);
    return storages;
}
std::vector<Storage> Storage::List(StorageType type)
{
    std::vector<Storage> storages;
    auto                 drives = GetAllLogicalDrives();
    DWORD                oldErr = 0;
    SetThreadErrorMode(SEM_FAILCRITICALERRORS, &oldErr);
    for (const auto& drive : drives)
    {
        auto storageType = ParseDriveType(drive);
        if (!storageType.has_value() || type != *storageType)
        {
            continue;
        }

        Storage storage;
        storage._impl->path.push_back(drive);
        storage._impl->path.push_back(':');
        storage._impl->type = *storageType;
        ParseStorageName(drive, storage._impl->name, storage._impl->fileSystem);
        GetStorageSpace(drive, storage._impl->totalCapacity, storage._impl->freeCapacity, storage._impl->availableCapacity);
        GetStorageLocation(drive, storage._impl->location);
        storages.emplace_back(std::move(storage));
    }
    SetThreadErrorMode(oldErr, nullptr);
    return storages;
}
} // namespace zeus::Hardware
#endif