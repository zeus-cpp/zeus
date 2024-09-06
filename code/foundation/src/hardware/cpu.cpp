#include "zeus/foundation/hardware/cpu.h"
#include "impl/cpu_impl.h"

namespace zeus::Hardware
{

Cpu::Cpu() : _impl(std::make_unique<CpuImpl>())
{
}
Cpu::~Cpu()
{
}
Cpu::Cpu(const Cpu& other) noexcept : _impl(std::make_unique<CpuImpl>(*other._impl))
{
}
Cpu::Cpu(Cpu&& other) noexcept : _impl(std::move(other._impl))
{
}

Cpu& Cpu::operator=(const Cpu& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

Cpu& Cpu::operator=(Cpu&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

std::string Cpu::Name() const noexcept
{
    return _impl->name;
}

size_t Cpu::PhysicalCoreCount() const noexcept
{
    return _impl->physicalCoreCount;
}

size_t Cpu::LogicalCoreCount() const noexcept
{
    return _impl->logicalCoreCount;
}
} // namespace zeus::Hardware