#pragma once
#include "zeus/foundation/hardware/cpu.h"

namespace zeus::Hardware
{

struct CpuImpl
{
    std::string name;
    size_t      physicalCoreCount = 0;
    size_t      logicalCoreCount  = 0;
};

}