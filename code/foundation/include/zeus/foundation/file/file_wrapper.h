#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <filesystem>
#include <chrono>
#include "zeus/expected.hpp"
#include "zeus/foundation/core/platform_def.h"

namespace zeus
{

struct FileWrapperImpl;
class FileWrapper
{
public:
    enum class OffsetType
    {
        kBegin,
        kCurrent,
        kEnd
    };
    enum class OpenMode
    {
        kRead,
        kWrite,
        kReadWrite
    };
    FileWrapper();
    FileWrapper(PlatformFileHandle file);
    FileWrapper(const FileWrapper&)            = delete;
    FileWrapper& operator=(const FileWrapper&) = delete;

    FileWrapper(PlatformFileHandleWrapper&& handle);
    FileWrapper& operator=(PlatformFileHandleWrapper&& handle) noexcept;

    FileWrapper(FileWrapper&& other) noexcept;
    FileWrapper& operator=(FileWrapper&& other) noexcept;
    ~FileWrapper();

#ifdef _WIN32
    operator HANDLE() const noexcept;
    HANDLE* operator&() noexcept;
    HANDLE  operator*() noexcept;
#endif
#ifdef __linux__
    explicit operator int() const noexcept;
    explicit operator int*() const noexcept;
#endif
    explicit operator bool() const noexcept;
    bool operator==(const PlatformFileHandleWrapper& handle) const noexcept;
    bool operator!=(const PlatformFileHandleWrapper& handle) const noexcept;
    bool operator==(PlatformFileHandle handle) const noexcept;
    bool operator!=(PlatformFileHandle handle) const noexcept;

    bool operator==(const FileWrapper& other) const noexcept;
    bool operator!=(const FileWrapper& other) const noexcept;

    void               Close() noexcept;
    bool               Empty() const noexcept;
    void               Attach(PlatformFileHandle handle) noexcept;
    PlatformFileHandle Detach() noexcept;
    PlatformFileHandle PlatformHandle() const noexcept;
#ifdef _WIN32
    HANDLE                                     Handle() const noexcept;
    zeus::expected<WinHandle, std::error_code> Duplicate(bool inherite = false, const std::optional<DWORD>& desiredAccess = std::nullopt) const;
    zeus::expected<void, std::error_code>      SetInherite(bool inherite);
    zeus::expected<bool, std::error_code>      IsInherited() const;
#endif
#ifdef __linux__
    int                                                  Fd() const noexcept;
    int                                                  FileDescriptor() const noexcept;
    zeus::expected<LinuxFileDescriptor, std::error_code> Duplicate(bool closeOnExec = true) const;
    zeus::expected<bool, std::error_code>                IsCloseOnExec() const;
    zeus::expected<void, std::error_code>                SetCloseOnExec(bool closeOnExec);
#endif
    zeus::expected<uint64_t, std::error_code>                              FileSize();
    zeus::expected<size_t, std::error_code>                                Write(const void* data, size_t size);
    zeus::expected<size_t, std::error_code>                                Write(const void* data, size_t size, int64_t offset, OffsetType type);
    zeus::expected<size_t, std::error_code>                                Write(const std::string& data);
    zeus::expected<size_t, std::error_code>                                Write(const std::string& data, int64_t offset, OffsetType type);
    zeus::expected<size_t, std::error_code>                                Write(std::string_view data);
    zeus::expected<size_t, std::error_code>                                Write(std::string_view data, int64_t offset, OffsetType type);
    zeus::expected<size_t, std::error_code>                                Read(void* buffer, size_t size);
    zeus::expected<size_t, std::error_code>                                Read(void* buffer, size_t size, int64_t offset, OffsetType type);
    zeus::expected<std::vector<uint8_t>, std::error_code>                  Read(size_t size);
    zeus::expected<std::vector<uint8_t>, std::error_code>                  Read(size_t size, int64_t offset, OffsetType type);
    zeus::expected<std::string, std::error_code>                           ReadString(size_t size);
    zeus::expected<std::string, std::error_code>                           ReadString(size_t size, int64_t offset, OffsetType type);
    zeus::expected<uint64_t, std::error_code>                              Seek(int64_t offset, OffsetType type);
    zeus::expected<uint64_t, std::error_code>                              Tell();
    zeus::expected<void, std::error_code>                                  Flush();
    zeus::expected<std::chrono::system_clock::time_point, std::error_code> CreateTime();
    zeus::expected<std::chrono::system_clock::time_point, std::error_code> LastAccessTime();
    zeus::expected<std::chrono::system_clock::time_point, std::error_code> LastWriteTime();
    zeus::expected<std::chrono::system_clock::time_point, std::error_code> LastChangeTime();

public:
    static zeus::expected<FileWrapper, std::error_code> Open(const std::filesystem::path& path, OpenMode mode);
    static zeus::expected<FileWrapper, std::error_code> Create(const std::filesystem::path& path, OpenMode mode, bool autoFlush = false);
    static zeus::expected<FileWrapper, std::error_code> OpenOrCreate(const std::filesystem::path& path, OpenMode mode, bool autoFlush = false);
    static zeus::expected<FileWrapper, std::error_code> Truncate(const std::filesystem::path& path, OpenMode mode, bool autoFlush = false);
    static zeus::expected<FileWrapper, std::error_code> OpenSymbolLink(const std::filesystem::path& path, OpenMode mode, bool autoFlush = false);
private:
    std::unique_ptr<FileWrapperImpl> _impl;
};

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
