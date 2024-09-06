#include "zeus/foundation/hardware/storage.h"
#ifdef __linux__
#include <filesystem>
#include <cstring>
#include <set>
#include <mntent.h>
#include <sys/statfs.h>
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/file/file_utils.h"
#include "impl/storage_impl.h"

namespace fs = std::filesystem;

namespace zeus::Hardware
{
namespace
{
Storage::StorageType ParseStorageType(const char* type, const std::string& deviceType)
{
    if (0 == strcmp(MNTTYPE_NFS, type))
    {
        return Storage::StorageType::kRemote;
    }
    else if (0 == strcmp(MNTTYPE_SWAP, type))
    {
        return Storage::StorageType::kRAM;
    }
    else
    {
        if (StartWith(deviceType, "sr") || StartWith(deviceType, "cdrom"))
        {
            return Storage::StorageType::kCD;
        }
        else if (StartWith(deviceType, "sd") || StartWith(deviceType, "hd") || StartWith(deviceType, "nvme") || StartWith(deviceType, "md"))
        {
            return Storage::StorageType::kDisk;
        }
        else if (StartWith(deviceType, "fd"))
        {
            return Storage::StorageType::kFloppy;
        }
        else
        {
            return Storage::StorageType::kUnknown;
        }
    }
}

uint64_t ParseTotalSize(const fs::path& block)
{
    static fs::path blockPrfix("/sys/class/block/");
    auto            sizeString = Trim(FileContent(blockPrfix / block / "size").value_or(""));
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

std::string GetLocation(const std::string& deviceType)
{
    if (StartWith(deviceType, "sd"))
    {
        return deviceType.substr(0, 3);
    }
    else if (StartWith(deviceType, "nvme"))
    {
        return deviceType.substr(0, 7);
    }
    else if (StartWith(deviceType, "md"))
    {
        return deviceType.substr(0, 3);
    }
    else if (StartWith(deviceType, "hd"))
    {
        return deviceType.substr(0, 3);
    }
    else
    {
        auto pos = deviceType.find_first_of("0123456789");
        if (std::string::npos != pos)
        {
            return deviceType.substr(0, pos + 1);
        }
        return "";
    }
}
} // namespace

std::vector<Storage> Storage::ListAll()
{
    std::vector<Storage>    storages;
    FILE*                   mtab      = setmntent(MOUNTED, "r");
    mntent                  mnt       = {};
    const size_t            bufferLen = 2048;
    std::unique_ptr<char[]> buffer    = std::make_unique<char[]>(bufferLen);
    std::memset(buffer.get(), 0, bufferLen);
    std::set<std::string> device;
    while (getmntent_r(mtab, &mnt, buffer.get(), bufferLen))
    {
        struct statfs64 stat = {};
        if (-1 == statfs64(mnt.mnt_dir, &stat) || 0 == stat.f_bsize * stat.f_blocks)
        {
            continue;
        }
        if (!zeus::StartWith(mnt.mnt_fsname, "/dev/"))
        {
            continue;
        }
        if (device.count(mnt.mnt_fsname))
        {
            continue;
        }
        else
        {
            device.emplace(mnt.mnt_fsname);
        }
        fs::path devicePath(mnt.mnt_fsname);
        fs::path mountPath(mnt.mnt_dir);
        auto     storageType = ParseStorageType(mnt.mnt_type, devicePath.filename().string());
        Storage  storage;
        storage._impl->path              = mnt.mnt_dir;
        storage._impl->type              = storageType;
        storage._impl->name              = mountPath.filename().string();
        storage._impl->availableCapacity = stat.f_bsize * stat.f_blocks / 1024;
        storage._impl->freeCapacity      = stat.f_bsize * stat.f_bavail / 1024;
        storage._impl->totalCapacity     = ParseTotalSize(devicePath.filename());
        storage._impl->fileSystem        = mnt.mnt_type;
        storage._impl->location          = GetLocation(devicePath.filename().string());
        storages.emplace_back(std::move(storage));
    }
    endmntent(mtab);
    return storages;
}
std::vector<Storage> Storage::List(StorageType type)
{
    std::vector<Storage>    storages;
    FILE*                   mtab      = setmntent(MOUNTED, "r");
    mntent                  mnt       = {};
    const size_t            bufferLen = 2048;
    std::unique_ptr<char[]> buffer    = std::make_unique<char[]>(bufferLen);
    std::memset(buffer.get(), 0, bufferLen);
    std::set<std::string> device;
    while (getmntent_r(mtab, &mnt, buffer.get(), bufferLen))
    {
        struct statfs64 stat = {};
        if (-1 == statfs64(mnt.mnt_dir, &stat) || 0 == stat.f_bsize * stat.f_blocks)
        {
            continue;
        }
        if (!zeus::StartWith(mnt.mnt_fsname, "/dev/"))
        {
            continue;
        }
        if (device.count(mnt.mnt_fsname))
        {
            continue;
        }
        else
        {
            device.emplace(mnt.mnt_fsname);
        }
        fs::path devicePath(mnt.mnt_fsname);
        fs::path mountPath(mnt.mnt_dir);
        auto     storageType = ParseStorageType(mnt.mnt_type, devicePath.filename().string());
        if (storageType != type)
        {
            continue;
        }
        Storage storage;
        storage._impl->path              = mnt.mnt_dir;
        storage._impl->type              = storageType;
        storage._impl->name              = mountPath.filename().string();
        storage._impl->availableCapacity = stat.f_bsize * stat.f_blocks / 1024;
        storage._impl->freeCapacity      = stat.f_bsize * stat.f_bavail / 1024;
        storage._impl->totalCapacity     = ParseTotalSize(devicePath.filename());
        storage._impl->fileSystem        = mnt.mnt_type;
        storage._impl->location          = GetLocation(devicePath.filename().string());
        storages.emplace_back(std::move(storage));
    }
    endmntent(mtab);
    return storages;
}
} // namespace zeus::Hardware
#endif
