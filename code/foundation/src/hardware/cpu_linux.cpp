#include "zeus/foundation/hardware/cpu.h"

#ifdef __linux__
#include <fstream>
#include <tuple>
#include <sstream>
#include <unistd.h>
#include "zeus/foundation/file/kv_file_utils.h"
#include "impl/cpu_impl.h"

namespace zeus::Hardware
{

using namespace std;
std::vector<Cpu> Cpu::ListAll()
{
    vector<Cpu> infos;
    Cpu         info;
    auto        cpuData = GetKVFileData("/proc/cpuinfo", ": ");
    if (!cpuData)
    {
        return infos;
    }
    {
        auto iter = cpuData->find("model name\t");
        if (iter != cpuData->end())
        {
            info._impl->name = iter->second;
        }
    }
    {
        auto iter = cpuData->find("cpu cores\t");
        if (iter != cpuData->end())
        {
            info._impl->physicalCoreCount = std::stoul(iter->second);
        }
    }
    {
        auto range = cpuData->equal_range("processor\t");
        if (range.first != cpuData->end())
        {
            info._impl->logicalCoreCount = std::distance(range.first, range.second);
        }
        if (0 == info._impl->physicalCoreCount)
        {
            info._impl->physicalCoreCount = info._impl->logicalCoreCount;
        }
    }
    infos.emplace_back(std::move(info));
    return infos;
}

namespace
{
std::tuple<uint64_t, uint64_t> GetCpuStat()
{
    ifstream file("/proc/stat");
    if (!file)
    {
        return {0, 0};
    }
    string line;
    std::getline(file, line);
    std::istringstream stream(line);
    std::string        title;
    uint64_t           user, nice, system, idle, iowait, irq, softirq;
    stream >> title >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    return {user + nice + system + idle + irq + softirq, idle};
}
}

double Cpu::GetUsagePercent(uint32_t sampleMillisecond)
{
    if (sampleMillisecond < 10)
    {
        // /proc/stat的统计周期单位是10ms，小于10ms的采样时间会导致统计错误
        sampleMillisecond = 10;
    }
    auto [total1, idle1] = GetCpuStat();
    usleep(sampleMillisecond * 1000);
    auto [total2, idle2] = GetCpuStat();
    auto diffTotal       = total2 - total1;
    auto diffIdle        = idle2 - idle1;
    if (diffTotal)
    {
        return 100.0 * (diffTotal - diffIdle) / diffTotal;
    }
    return 0.0;
}

} // namespace zeus::Hardware
#endif