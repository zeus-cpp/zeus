#include "zeus/foundation/hardware/storage.h"
#include <set>
#include "zeus/foundation/hardware/hard_disk.h"
#include "impl/storage_impl.h"

namespace zeus::Hardware
{
Storage::Storage() : _impl(std::make_unique<StorageImpl>())
{
}
Storage::~Storage()
{
}
Storage::Storage(const Storage& other) : _impl(std::make_unique<StorageImpl>(*other._impl))
{
}
Storage::Storage(Storage&& other) noexcept : _impl(std::move(other._impl))
{
}
Storage& Storage::operator=(const Storage& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
Storage& Storage::operator=(Storage&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string Storage::Path() const
{
    return _impl->path;
}
std::string Storage::Name() const
{
    return _impl->name;
}
Storage::StorageType Storage::Type() const
{
    return _impl->type;
}
uint64_t Storage::TotalCapacity() const
{
    return _impl->totalCapacity;
}
uint64_t Storage::AvailableCapacity() const
{
    return _impl->availableCapacity;
}
uint64_t Storage::FreeCapacity() const
{
    return _impl->freeCapacity;
}
std::string Storage::FileSystem() const
{
    return _impl->fileSystem;
}
std::string Storage::Location() const
{
    return _impl->location;
}

std::vector<Storage> Storage::ListRemovableStorage(bool removable)
{
    std::vector<Storage>  removableDisks;
    auto                  removableHardDisks = HardDisk::ListRemovable(removable);
    auto                  storages           = List(StorageType::kDisk);
    std::set<std::string> removableDiskLocation;
    for (const auto& disk : removableHardDisks)
    {
        removableDiskLocation.emplace(disk.Location());
    }
    for (auto& storage : storages)
    {
        if (removableDiskLocation.find(storage.Location()) != removableDiskLocation.end())
        {
            removableDisks.emplace_back(std::move(storage));
        }
    }
    return removableDisks;
}

std::string Storage::TypeName(StorageType type)
{
    switch (type)
    {
    case StorageType::kDisk:
        return "Disk";
    case StorageType::kCD:
        return "CD";
    case StorageType::kRAM:
        return "RAM";
    case StorageType::kFloppy:
        return "Floppy";
    case StorageType::kRemote:
        return "Remote";
    default:
        return "Unknown";
        break;
    }
}
} // namespace zeus::Hardware