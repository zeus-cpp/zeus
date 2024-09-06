#pragma once

#include "zeus/foundation/hardware/storage.h"
namespace zeus::Hardware
{
struct StorageImpl
{
    std::string          path;
    std::string          name;
    Storage::StorageType type              = Storage::StorageType::kUnknown;
    uint64_t             totalCapacity     = 0;
    uint64_t             availableCapacity = 0;
    uint64_t             freeCapacity      = 0;
    std::string          fileSystem;
    std::string          location;
};
} // namespace zeus::Hardware