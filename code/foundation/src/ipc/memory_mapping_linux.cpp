#include "zeus/foundation/ipc/memory_mapping.h"
#ifdef __linux__

#include <unistd.h>
#include <sys/mman.h>
#include "zeus/foundation/core/system_error.h"

namespace zeus
{

struct MemoryMappingImpl
{
    void*    baseAddress = nullptr;
    uint64_t mapSize     = 0;
    void*    data        = nullptr;
    uint64_t size        = 0;
};

MemoryMapping::MemoryMapping() : _impl(std::make_unique<MemoryMappingImpl>())
{
}

MemoryMapping::MemoryMapping(MemoryMapping&& other) noexcept : _impl(std::move(other._impl))
{
}

MemoryMapping& MemoryMapping::operator=(MemoryMapping&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
MemoryMapping::~MemoryMapping()
{
    if (_impl)
    {
        if (_impl->baseAddress)
        {
            munmap(_impl->baseAddress, _impl->mapSize);
        }
    }
}

bool MemoryMapping::operator==(const MemoryMapping& other) const
{
    return _impl->data == other._impl->data && _impl->size == other._impl->size;
}

bool MemoryMapping::operator!=(const MemoryMapping& other) const
{
    return !(*this == other);
}

bool MemoryMapping::operator==(const void* data) const
{
    return _impl->data == data;
}

bool MemoryMapping::operator!=(const void* data) const
{
    return !(*this == data);
}

MemoryMapping::operator bool() const
{
    return _impl->data != nullptr;
}

bool MemoryMapping::Empty() const
{
    return _impl->data == nullptr;
}

void* MemoryMapping::Data() const
{
    return _impl->data;
}

uint64_t MemoryMapping::Size() const
{
    return _impl->size;
}

zeus::expected<void, std::error_code> MemoryMapping::UnMap()
{
    if (0 == munmap(_impl->baseAddress, _impl->mapSize))
    {
        _impl->baseAddress = nullptr;
        _impl->mapSize     = 0;
        _impl->data        = nullptr;
        _impl->size        = 0;
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

size_t MemoryMapping::SystemMemoryAlign()
{
    return sysconf(_SC_PAGESIZE);
}

zeus::expected<MemoryMapping, std::error_code> MemoryMapping::Map(PlatformMemoryMappingHandle handle, uint64_t size, uint64_t offset, bool readOnly)
{
    const auto align = offset % SystemMemoryAlign();
    void*      data  = mmap(nullptr, size + align, readOnly ? PROT_READ : (PROT_READ | PROT_WRITE), MAP_SHARED, handle, offset - align);
    if (MAP_FAILED == data)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    MemoryMapping mapping;
    mapping._impl->baseAddress = data;
    mapping._impl->mapSize     = size + align;
    mapping._impl->data        = static_cast<uint8_t*>(data) + align;
    mapping._impl->size        = size;
    return mapping;
}

zeus::expected<void, std::error_code> zeus::MemoryMapping::Flush()
{
    if (0 == msync(_impl->baseAddress, _impl->mapSize, MS_SYNC))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}
} // namespace zeus
#endif