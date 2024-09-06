#include "zeus/foundation/hardware/memory.h"
#ifdef __linux
#include <fstream>
#include <string>
#include "zeus/foundation/file/kv_file_utils.h"
#include "impl/memory_impl.h"

namespace
{
// linux下除了DMI信息以外，没有可以得知安装内存容量的方法，但是DMI需要ROOT，所以只能通过将总可见内存按照GB向上取整来推测安装容量
uint64_t GuessInstalledMamory(uint64_t visiblePhysicalCapacity)
{
    return (visiblePhysicalCapacity + 1024 * 1024 - 1) / 1024 / 1024 * 1024 * 1024;
}
}

namespace zeus::Hardware
{
Memory Memory::GetMemory(bool bank)
{
    Memory result;
    result._impl->visiblePhysicalCapacity = std::stoul(GetKVFileValue("/proc/meminfo", "MemTotal", ":").value_or("0"));
    result._impl->totalPageCapacity       = std::stoul(GetKVFileValue("/proc/meminfo", "SwapTotal", ":").value_or("0"));
    auto available                        = std::stoul(GetKVFileValue("/proc/meminfo", "MemAvailable", ":").value_or("0"));
    if (result._impl->visiblePhysicalCapacity > available)
    {
        result._impl->freePhysicalCapacity = result._impl->visiblePhysicalCapacity - available;
    }
    else
    {
        result._impl->freePhysicalCapacity = 0;
    }
    if (bank)
    {
        result._impl->bankList = MemoryBank::GetMemoryBankList();
        for (const auto& memoryBank : result._impl->bankList)
        {
            result._impl->installedPhysicalCapacity += memoryBank.Capacity();
        }
    }
    if (0 == result._impl->installedPhysicalCapacity)
    {
        result._impl->installedPhysicalCapacity = GuessInstalledMamory(result._impl->visiblePhysicalCapacity);
    }
    return result;
}
} // namespace zeus::Hardware

#endif