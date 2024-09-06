#pragma once
#include <memory>
#include <cstdint>
#include <system_error>
#include "zeus/expected.hpp"
#include "zeus/foundation/core/platform_def.h"

namespace zeus
{
struct MemoryMappingImpl;
class MemoryMapping
{
public:
    MemoryMapping(const MemoryMapping &)            = delete;
    MemoryMapping &operator=(const MemoryMapping &) = delete;
    MemoryMapping(MemoryMapping &&other) noexcept;
    MemoryMapping &operator=(MemoryMapping &&other) noexcept;
    ~MemoryMapping();
    bool operator==(const MemoryMapping &other) const;
    bool operator!=(const MemoryMapping &other) const;
    bool operator==(const void *data) const;
    bool operator!=(const void *data) const;
    operator bool() const;
    bool                                  Empty() const;
    void                                 *Data() const;
    uint64_t                              Size() const;
    zeus::expected<void, std::error_code> UnMap();
    zeus::expected<void, std::error_code> Flush();
public:
    static size_t                                         SystemMemoryAlign();
    static zeus::expected<MemoryMapping, std::error_code> Map(
        PlatformMemoryMappingHandle handle, uint64_t size, uint64_t offset = 0, bool readOnly = false
    );
protected:
    MemoryMapping();
private:
    std::unique_ptr<MemoryMappingImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
