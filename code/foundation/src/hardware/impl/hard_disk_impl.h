#pragma once

#include "zeus/foundation/hardware/hard_disk.h"
namespace zeus::Hardware
{
struct HardDiskImpl
{
    std::string               model;
    std::string               serialNumber;
    uint64_t                  capacity    = 0;
    bool                      isSSD       = false;
    bool                      isRemovable = false;
    std::string               location;
    HardDisk::HardDiskBusType busType = HardDisk::HardDiskBusType::kUnknown;
};
} // namespace zeus::Hardware