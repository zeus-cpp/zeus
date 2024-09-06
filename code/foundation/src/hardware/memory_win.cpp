#include "zeus/foundation/hardware/memory.h"
#ifdef _WIN32
#include <windows.h>
#include "impl/memory_impl.h"

namespace zeus::Hardware
{
Memory Memory::GetMemory(bool bank)
{
    Memory         result;
    MEMORYSTATUSEX state = {};
    state.dwLength       = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&state))
    {
        result._impl->totalPageCapacity       = (state.ullTotalPageFile - state.ullTotalPhys) / 1024;
        result._impl->visiblePhysicalCapacity = state.ullTotalPhys / 1024;
        result._impl->freePhysicalCapacity    = state.ullAvailPhys / 1024;
    }
    ULONGLONG installedMemory = 0;
    if (GetPhysicallyInstalledSystemMemory(&installedMemory))
    {
        result._impl->installedPhysicalCapacity = installedMemory;
    }
    if (bank || 0 == result._impl->installedPhysicalCapacity)
    {
        result._impl->bankList = MemoryBank::GetMemoryBankList();
        if (0 == result._impl->installedPhysicalCapacity)
        {
            for (const auto& memoryBank : result._impl->bankList)
            {
                result._impl->installedPhysicalCapacity += memoryBank.Capacity();
            }
        }
    }
    return result;
}

} // namespace zeus::Hardware

#endif