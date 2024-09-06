#include "zeus/foundation/hardware/cpu.h"
#ifdef _WIN32
#include <Windows.h>
#include "zeus/foundation/system/win/registry.h"
#include "zeus/foundation/system/environment.h"
#include "impl/cpu_impl.h"

namespace zeus
{
namespace Hardware
{

std::vector<Cpu> Cpu::ListAll()
{
    std::vector<Cpu> infos;
    Cpu              cpu;
    auto             processor0 = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", true);
    if (processor0)
    {
        cpu._impl->name = processor0->GetStringValue("ProcessorNameString").value_or("");
    }
    cpu._impl->physicalCoreCount = zeus::Environment::ProcessorCoreCount();
    cpu._impl->logicalCoreCount  = zeus::Environment::ProcessorCount();

    infos.emplace_back(std::move(cpu));
    return infos;
}

namespace
{
uint64_t FileTimeToInt(const FILETIME& ft)
{
    ULARGE_INTEGER uli = {};
    uli.LowPart        = ft.dwLowDateTime;
    uli.HighPart       = ft.dwHighDateTime;
    return uli.QuadPart;
}
}

double Cpu::GetUsagePercent(uint32_t sampleMillisecond)
{
    FILETIME preIdleTime, idleTime;

    FILETIME preKernelTime, kernelTime;

    FILETIME preUserTime, userTime;
    GetSystemTimes(&preIdleTime, &preKernelTime, &preUserTime);
    Sleep(sampleMillisecond);
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    auto idle   = FileTimeToInt(idleTime) - FileTimeToInt(preIdleTime);
    auto kernel = FileTimeToInt(kernelTime) - FileTimeToInt(preKernelTime);
    auto user   = FileTimeToInt(userTime) - FileTimeToInt(preUserTime);
    auto usage  = (kernel + user - idle) * 100.0 / (kernel + user);
    if (usage > 100.0)
    {
        return 100.0;
    }
    else
    {
        return usage;
    }
}
} // namespace Hardware
} // namespace zeus
#endif