#pragma once

#include <memory>
#include <string>
#include <vector>

namespace zeus
{
namespace Hardware
{
struct CpuImpl;
class Cpu
{
public:
    Cpu();
    ~Cpu();
    Cpu(const Cpu& other) noexcept;
    Cpu(Cpu&& other) noexcept;
    Cpu&        operator=(const Cpu& other) noexcept;
    Cpu&        operator=(Cpu&& other) noexcept;
    std::string Name() const noexcept;
    size_t      PhysicalCoreCount() const noexcept;
    size_t      LogicalCoreCount() const noexcept;

    //实现永远只返回一个cpu，因为暂时无法找到多cpu的测试设备
public:
    static std::vector<Cpu> ListAll();
    //此函数需要计算两次CPU采样，需要自己指定采样时间，默认采样1000ms，结果为0-100.0
    static double           GetUsagePercent(uint32_t sampleMillisecond = 1000);
private:
    std::unique_ptr<CpuImpl> _impl;
};
} // namespace Hardware
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
