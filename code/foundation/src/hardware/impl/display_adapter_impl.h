#include "zeus/foundation/hardware/display_adapter.h"
#include <string>
#include <cstdint>

namespace zeus::Hardware
{
struct DisplayAdapterImpl
{
    uint64_t    systemMemory = 0; // KB
    uint64_t    videoMemory  = 0; // KB
    uint64_t    sharedMemory = 0; // KB
    std::string describeName;
    uint16_t    vendorId = 0;
};
}