#ifdef _WIN32
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/system/win/file_attributes.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/time/time_utils.h"

namespace zeus
{

struct WinFileAttributesImpl
{
    DWORD                                 attributes = INVALID_FILE_ATTRIBUTES;
    uint64_t                              fileSize   = 0;
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point lastAccessTime;
    std::chrono::system_clock::time_point lastWriteTime;
    std::filesystem::path                 path;
};

namespace
{
zeus::expected<void, std::error_code> RefreshAttributes(WinFileAttributesImpl& impl)
{
    WIN32_FILE_ATTRIBUTE_DATA data {};
    if (!GetFileAttributesExW(impl.path.c_str(), GetFileExInfoStandard, &data))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    impl.attributes     = data.dwFileAttributes;
    impl.fileSize       = (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
    impl.creationTime   = FiletimeToSystemTime(data.ftCreationTime);
    impl.lastAccessTime = FiletimeToSystemTime(data.ftLastAccessTime);
    impl.lastWriteTime  = FiletimeToSystemTime(data.ftLastWriteTime);
    return {};
}
}

WinFileAttributes::WinFileAttributes() : _impl(std::make_unique<WinFileAttributesImpl>())
{
}
WinFileAttributes::WinFileAttributes(WinFileAttributes&& other) noexcept : _impl(std::move(other._impl))
{
}
WinFileAttributes& WinFileAttributes::operator=(WinFileAttributes&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
WinFileAttributes::~WinFileAttributes()
{
}

bool WinFileAttributes::IsNormal() const
{
    return FILE_ATTRIBUTE_NORMAL & _impl->attributes;
}

bool WinFileAttributes::IsReadOnly() const
{
    return FILE_ATTRIBUTE_READONLY & _impl->attributes;
}
bool WinFileAttributes::IsHidden() const
{
    return FILE_ATTRIBUTE_HIDDEN & _impl->attributes;
}
bool WinFileAttributes::IsCompress() const
{
    return FILE_ATTRIBUTE_COMPRESSED & _impl->attributes;
}
bool WinFileAttributes::IsEncrypted() const
{
    return FILE_ATTRIBUTE_ENCRYPTED & _impl->attributes;
}
bool WinFileAttributes::IsSystem() const
{
    return FILE_ATTRIBUTE_SYSTEM & _impl->attributes;
}
bool WinFileAttributes::IsArchive() const
{
    return FILE_ATTRIBUTE_ARCHIVE & _impl->attributes;
}
bool WinFileAttributes::IsTemporary() const
{
    return FILE_ATTRIBUTE_TEMPORARY & _impl->attributes;
}
bool WinFileAttributes::IsDirectory() const
{
    return FILE_ATTRIBUTE_DIRECTORY & _impl->attributes;
}
bool WinFileAttributes::IsSymbolicLink() const
{
    return FILE_ATTRIBUTE_REPARSE_POINT & _impl->attributes;
}
uint64_t WinFileAttributes::GetSize() const
{
    return _impl->fileSize;
}
std::chrono::system_clock::time_point WinFileAttributes::GetCreationTime() const
{
    return _impl->creationTime;
}
std::chrono::system_clock::time_point WinFileAttributes::GetLastAccessTime() const
{
    return _impl->lastAccessTime;
}
std::chrono::system_clock::time_point WinFileAttributes::GetLastWriteTime() const
{
    return _impl->lastWriteTime;
}
zeus::expected<void, std::error_code> WinFileAttributes::SetNormal()
{
    if (SetFileAttributesW(_impl->path.c_str(), FILE_ATTRIBUTE_NORMAL))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetReadOnly(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_READONLY) : (_impl->attributes & ~FILE_ATTRIBUTE_READONLY);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetHidden(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_HIDDEN) : (_impl->attributes & ~FILE_ATTRIBUTE_HIDDEN);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetCompress(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_COMPRESSED) : (_impl->attributes & ~FILE_ATTRIBUTE_COMPRESSED);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetEncrypted(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_ENCRYPTED) : (_impl->attributes & ~FILE_ATTRIBUTE_ENCRYPTED);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetSystem(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_SYSTEM) : (_impl->attributes & ~FILE_ATTRIBUTE_SYSTEM);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetArchive(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_ARCHIVE) : (_impl->attributes & ~FILE_ATTRIBUTE_ARCHIVE);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<void, std::error_code> WinFileAttributes::SetTemporary(bool state)
{
    const auto attributes = state ? (_impl->attributes | FILE_ATTRIBUTE_TEMPORARY) : (_impl->attributes & ~FILE_ATTRIBUTE_TEMPORARY);
    if (SetFileAttributesW(_impl->path.c_str(), attributes))
    {
        return RefreshAttributes(*_impl);
    }
    return zeus::unexpected(GetLastSystemError());
}
zeus::expected<WinFileAttributes, std::error_code> WinFileAttributes::Load(const std::filesystem::path& path)
{
    std::wstring filePath = path.wstring();
    if (!StartWith(filePath, LR"(\)") && filePath.size() >= MAX_PATH)
    {
        filePath = LR"(\\?\)" + filePath;
    }
    WIN32_FILE_ATTRIBUTE_DATA data {};
    if (!GetFileAttributesExW(filePath.c_str(), GetFileExInfoStandard, &data))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WinFileAttributes fileAttributes;
    fileAttributes._impl->path           = filePath;
    fileAttributes._impl->attributes     = data.dwFileAttributes;
    fileAttributes._impl->fileSize       = (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
    fileAttributes._impl->creationTime   = FiletimeToSystemTime(data.ftCreationTime);
    fileAttributes._impl->lastAccessTime = FiletimeToSystemTime(data.ftLastAccessTime);
    fileAttributes._impl->lastWriteTime  = FiletimeToSystemTime(data.ftLastWriteTime);
    return fileAttributes;
}
} // namespace zeus

#endif
