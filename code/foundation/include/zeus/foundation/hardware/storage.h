#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace zeus::Hardware
{
struct StorageImpl;
class Storage
{
public:
    enum class StorageType
    {
        kUnknown,
        kDisk,
        kCD,
        kRAM,
        kFloppy,
        kRemote,
    };
    Storage();
    ~Storage();
    Storage(const Storage& other);
    Storage(Storage&& other) noexcept;
    Storage&    operator=(const Storage& other);
    Storage&    operator=(Storage&& other) noexcept;
    std::string Path() const;
    std::string Name() const;
    StorageType Type() const;
    uint64_t    TotalCapacity() const; //kb，总容量,包括文件系统索引的空间
    uint64_t    AvailableCapacity(
       ) const; //kb,可用容量，不是指空闲还能写入文件的容量,是指总容量减去文件系统索引(Super Block)等不可以被使用的空间，即可重复使用的空间，windows下AvailableCapacity=TotalCapacity
    uint64_t    FreeCapacity() const; //kb，空闲还可以写入文件的容量
    std::string FileSystem() const;
    std::string Location() const; //对应硬件设备的位置信息

public:
    static std::vector<Storage> ListAll();
    static std::vector<Storage> List(StorageType type);
    static std::vector<Storage> ListRemovableStorage(bool removable);
    static std::string          TypeName(StorageType type);
private:
    std::unique_ptr<StorageImpl> _impl;
};
} // namespace zeus::Hardware
#include "zeus/foundation/core/zeus_compatible.h"
