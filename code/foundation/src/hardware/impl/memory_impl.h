#pragma once
#include "zeus/foundation/hardware/memory.h"
namespace zeus::Hardware
{

struct MemoryImpl
{
    uint64_t                totalPageCapacity         = 0; //KB
    uint64_t                visiblePhysicalCapacity   = 0; //KB
    uint64_t                installedPhysicalCapacity = 0; //KB
    uint64_t                freePhysicalCapacity      = 0; //KB
    std::vector<MemoryBank> bankList;
};
}