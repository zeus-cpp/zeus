#include "zeus/foundation/ipc/memory_mapping.h"
#ifdef _WIN32
#include <Windows.h>
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
            UnmapViewOfFile(_impl->baseAddress);
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

zeus::expected<void, std::error_code> zeus::MemoryMapping::UnMap()
{
    if (UnmapViewOfFile(_impl->baseAddress))
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
    SYSTEM_INFO info = {};
    GetSystemInfo(&info);
    return info.dwAllocationGranularity;
}

zeus::expected<MemoryMapping, std::error_code> MemoryMapping::Map(PlatformMemoryMappingHandle handle, uint64_t size, uint64_t offset, bool readOnly)
{
    const auto    align         = offset % SystemMemoryAlign();
    LARGE_INTEGER offsetInteger = {};
    offsetInteger.QuadPart      = offset - align;
    void* data = MapViewOfFile(handle, (readOnly ? 0 : FILE_MAP_WRITE) | FILE_MAP_READ, offsetInteger.HighPart, offsetInteger.LowPart, size + align);
    if (!data)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    MemoryMapping mapping;
    mapping._impl->baseAddress = data;
    mapping._impl->mapSize     = size + align;
    mapping._impl->data        = static_cast<uint8_t*>(data) + align;
    mapping._impl->size        = size;
    return std::move(mapping);
}

zeus::expected<void, std::error_code> zeus::MemoryMapping::Flush()
{
    if (FlushViewOfFile(_impl->baseAddress, _impl->mapSize))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}
} // namespace zeus
#endif