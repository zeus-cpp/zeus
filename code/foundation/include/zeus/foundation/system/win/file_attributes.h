#pragma once
#ifdef _WIN32
#include <memory>
#include <string>
#include <filesystem>
#include <chrono>
#include <system_error>
#include <cstdint>
#include <zeus/expected.hpp>

namespace zeus
{
struct WinFileAttributesImpl;
class WinFileAttributes
{
public:
    WinFileAttributes(const WinFileAttributes&) = delete;
    WinFileAttributes(WinFileAttributes&& other) noexcept;
    WinFileAttributes& operator=(const WinFileAttributes&) = delete;
    WinFileAttributes& operator=(WinFileAttributes&& other) noexcept;
    ~WinFileAttributes();
    bool                                  IsNormal() const;
    bool                                  IsReadOnly() const;
    bool                                  IsHidden() const;
    bool                                  IsCompress() const;
    bool                                  IsEncrypted() const;
    bool                                  IsSystem() const;
    bool                                  IsArchive() const;
    bool                                  IsTemporary() const;
    bool                                  IsDirectory() const;
    bool                                  IsSymbolicLink() const;
    uint64_t                              GetSize() const;
    std::chrono::system_clock::time_point GetCreationTime() const;
    std::chrono::system_clock::time_point GetLastAccessTime() const;
    std::chrono::system_clock::time_point GetLastWriteTime() const;

    zeus::expected<void, std::error_code> SetNormal();
    zeus::expected<void, std::error_code> SetReadOnly(bool state);
    zeus::expected<void, std::error_code> SetHidden(bool state);
    zeus::expected<void, std::error_code> SetCompress(bool state);
    zeus::expected<void, std::error_code> SetEncrypted(bool state);
    zeus::expected<void, std::error_code> SetSystem(bool state);
    zeus::expected<void, std::error_code> SetArchive(bool state);
    zeus::expected<void, std::error_code> SetTemporary(bool state);
public:
    static zeus::expected<WinFileAttributes, std::error_code> Load(const std::filesystem::path& path);
protected:
    WinFileAttributes();
private:
    std::unique_ptr<WinFileAttributesImpl> _impl;
};
} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
