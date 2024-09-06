#pragma once
#include <memory>
#include <filesystem>
#include <string>
#include <cstdint>
#include <zeus/expected.hpp>
#include "zeus/foundation/core/platform_def.h"

namespace zeus
{
struct FileMappingImpl;
class FileMapping
{
public:
    FileMapping(const FileMapping&)            = delete;
    FileMapping& operator=(const FileMapping&) = delete;
    FileMapping(FileMapping&& other) noexcept;
    FileMapping& operator=(FileMapping&& other) noexcept;
    ~FileMapping();
    zeus::expected<void, std::error_code> Map(uint64_t offset, uint64_t length);
    zeus::expected<void, std::error_code> MapAll(uint64_t offset = 0);
    bool                                  Empty() const;
    void*                                 Data() const;
    uint64_t                              Size() const;
    uint64_t                              FileSize() const;
    zeus::expected<void, std::error_code> UnMap();
    zeus::expected<void, std::error_code> Flush();
public:
    static zeus::expected<FileMapping, std::error_code> Create(const std::filesystem::path& path, bool writable);
    static zeus::expected<FileMapping, std::error_code> Create(PlatformFileHandle file, bool writable);

protected:
    FileMapping();
private:
    std::unique_ptr<FileMappingImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
