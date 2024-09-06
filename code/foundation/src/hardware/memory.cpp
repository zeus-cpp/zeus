#include "zeus/foundation/hardware/memory.h"
#include "impl/memory_impl.h"

namespace zeus::Hardware
{

Memory::Memory() : _impl(std::make_unique<MemoryImpl>())
{
}
Memory::~Memory()
{
}
Memory::Memory(const Memory& other) noexcept : _impl(std::make_unique<MemoryImpl>(*other._impl))
{
}
Memory::Memory(Memory&& other) noexcept : _impl(std::move(other._impl))
{
}

Memory& Memory::operator=(const Memory& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

Memory& Memory::operator=(Memory&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

uint64_t Memory::TotalPageCapacity() const noexcept
{
    return _impl->totalPageCapacity;
}
uint64_t Memory::VisiblePhysicalCapacity() const noexcept
{
    return _impl->visiblePhysicalCapacity;
}
uint64_t Memory::InstalledPhysicalCapacity() const noexcept
{
    return _impl->installedPhysicalCapacity;
}
uint64_t Memory::FreePhysicalCapacity() const noexcept
{
    return _impl->freePhysicalCapacity;
}
const std::vector<MemoryBank>& Memory::BankList() const noexcept
{
    return _impl->bankList;
}

} // namespace zeus::Hardware
