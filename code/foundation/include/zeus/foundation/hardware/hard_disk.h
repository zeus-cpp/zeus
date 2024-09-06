#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace zeus::Hardware
{
struct HardDiskImpl;
class HardDisk
{
public:
    enum class HardDiskBusType
    {
        kUnknown,
        kUSB,
        kATA,
        kSD,
        kSAS,
        kSATA,
        kSCSI,
        kNVMe,
        kRAID,
        kVirtual,
    };
    HardDisk();
    ~HardDisk();
    HardDisk(const HardDisk& other);
    HardDisk(HardDisk&& other) noexcept;
    HardDisk&       operator=(const HardDisk& other);
    HardDisk&       operator=(HardDisk&& other) noexcept;
    std::string     Model() const;
    std::string     SerialNumber() const;
    uint64_t        Capacity() const; //kb
    bool            IsSSD() const;
    bool            IsRemovable() const;
    std::string     Location() const;
    HardDiskBusType BusType() const;
public:
    static std::vector<HardDisk> ListAll();
    static std::vector<HardDisk> ListRemovable(bool removable);
    static std::string           BusTypeName(HardDiskBusType type);
private:
    std::unique_ptr<HardDiskImpl> _impl;
};
} // namespace zeus::Hardware
#include "zeus/foundation/core/zeus_compatible.h"
