#include "zeus/foundation/file/file_wrapper.h"
#ifdef _WIN32
#include <cassert>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/time/time_utils.h"

namespace zeus
{
namespace
{
DWORD OffsetTypeToMoveMethod(FileWrapper::OffsetType type)
{
    switch (type)
    {
    case FileWrapper::OffsetType::kBegin:
        return FILE_BEGIN;
    case FileWrapper::OffsetType::kCurrent:
        return FILE_CURRENT;
    case FileWrapper::OffsetType::kEnd:
        return FILE_END;
    default:
        assert(false);
        return FILE_CURRENT;
    }
}

DWORD ModeToAccess(FileWrapper::OpenMode mode)
{
    switch (mode)
    {
    case FileWrapper::OpenMode::kRead:
        return GENERIC_READ;
    case FileWrapper::OpenMode::kWrite:
        return GENERIC_WRITE;
    case FileWrapper::OpenMode::kReadWrite:
        return GENERIC_READ | GENERIC_WRITE;
    default:
        assert(false);
        return GENERIC_READ | GENERIC_WRITE;
    }
}

zeus::expected<FileWrapper, std::error_code> CreateFileWrapper(
    const std::filesystem::path& path, FileWrapper::OpenMode mode, DWORD disposition, DWORD attribute, bool autoFlush
)
{
    SECURITY_ATTRIBUTES securittyAttr = {};
    securittyAttr.nLength             = sizeof(SECURITY_ATTRIBUTES);
    securittyAttr.bInheritHandle      = FALSE;
    SECURITY_DESCRIPTOR sd            = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    SetSecurityDescriptorGroup(&sd, nullptr, FALSE);
    SetSecurityDescriptorSacl(&sd, FALSE, nullptr, FALSE);
    securittyAttr.lpSecurityDescriptor = &sd;
    if (autoFlush)
    {
        attribute |= FILE_FLAG_WRITE_THROUGH;
    }
    HANDLE handle =
        CreateFileW(path.c_str(), ModeToAccess(mode), FILE_SHARE_READ | FILE_SHARE_WRITE, &securittyAttr, disposition, attribute, nullptr);
    if (INVALID_HANDLE_VALUE == handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return FileWrapper(handle);
}
} // namespace

struct FileWrapperImpl
{
    WinHandle handle;
};

FileWrapper::FileWrapper() : _impl(std::make_unique<FileWrapperImpl>())
{
}

FileWrapper::FileWrapper(PlatformFileHandle handle) : _impl(std::make_unique<FileWrapperImpl>(FileWrapperImpl {handle}))
{
}

FileWrapper::FileWrapper(PlatformFileHandleWrapper&& handle) : _impl(std::make_unique<FileWrapperImpl>())
{
    _impl->handle = std::move(handle);
}
FileWrapper& FileWrapper::operator=(PlatformFileHandleWrapper&& handle) noexcept
{
    _impl->handle = std::move(handle);
    return *this;
}
FileWrapper::FileWrapper(FileWrapper&& other) noexcept : _impl(std::move(other._impl))
{
}

FileWrapper& FileWrapper::operator=(FileWrapper&& other) noexcept
{
    if (this != std::addressof(other))
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
FileWrapper::~FileWrapper()
{
}

FileWrapper::operator HANDLE() const noexcept
{
    return _impl->handle;
}

PlatformFileHandle* FileWrapper::operator&() noexcept
{
    return &_impl->handle;
}
PlatformFileHandle FileWrapper::operator*() noexcept
{
    return _impl->handle;
}
FileWrapper::operator bool() const noexcept
{
    return !Empty();
}
bool FileWrapper::operator==(const PlatformFileHandleWrapper& handle) const noexcept
{
    return _impl->handle == handle;
}
bool FileWrapper::operator!=(const PlatformFileHandleWrapper& handle) const noexcept
{
    return _impl->handle != handle;
}
bool FileWrapper::operator==(PlatformFileHandle handle) const noexcept
{
    return _impl->handle == handle;
}
bool FileWrapper::operator!=(PlatformFileHandle handle) const noexcept
{
    return _impl->handle != handle;
}

bool FileWrapper::operator==(const FileWrapper& other) const noexcept
{
    return _impl->handle == other._impl->handle;
}
bool FileWrapper::operator!=(const FileWrapper& other) const noexcept
{
    return _impl->handle != other._impl->handle;
}

void FileWrapper::Close() noexcept
{
    _impl->handle.Close();
}
bool FileWrapper::Empty() const noexcept
{
    return _impl->handle.Empty();
}
void FileWrapper::Attach(PlatformFileHandle handle) noexcept
{
    _impl->handle.Attach(handle);
}
PlatformFileHandle FileWrapper::Detach() noexcept
{
    return _impl->handle.Detach();
}

PlatformFileHandle FileWrapper::PlatformHandle() const noexcept
{
    return _impl->handle;
}

HANDLE FileWrapper::Handle() const noexcept
{
    return _impl->handle;
}
zeus::expected<WinHandle, std::error_code> FileWrapper::Duplicate(bool inherite, const std::optional<DWORD>& desiredAccess) const
{
    return _impl->handle.Duplicate(inherite, desiredAccess);
}
zeus::expected<void, std::error_code> FileWrapper::SetInherite(bool inherite)
{
    return _impl->handle.SetInherite(inherite);
}
zeus::expected<bool, std::error_code> FileWrapper::IsInherited() const
{
    return _impl->handle.IsInherited();
}

zeus::expected<uint64_t, std::error_code> FileWrapper::FileSize()
{
    assert(!Empty());
    LARGE_INTEGER size;
    if (!GetFileSizeEx(_impl->handle, &size))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return size.QuadPart;
}
zeus::expected<size_t, std::error_code> FileWrapper::Write(const void* data, size_t size)
{
    assert(!Empty());
    DWORD written = 0;
    if (!WriteFile(_impl->handle, data, size, &written, nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return written;
}

zeus::expected<size_t, std::error_code> FileWrapper::Read(void* buffer, size_t size)
{
    assert(!Empty());
    DWORD read = 0;
    if (!ReadFile(_impl->handle, buffer, size, &read, nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return read;
}
zeus::expected<uint64_t, std::error_code> FileWrapper::Seek(int64_t offset, OffsetType type)
{
    assert(!Empty());
    LARGE_INTEGER off = {};
    off.QuadPart      = offset;
    if (!SetFilePointerEx(_impl->handle, off, &off, OffsetTypeToMoveMethod(type)))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return off.QuadPart;
}
zeus::expected<uint64_t, std::error_code> FileWrapper::Tell()
{
    assert(!Empty());
    LARGE_INTEGER off = {};
    if (!SetFilePointerEx(_impl->handle, {}, &off, FILE_CURRENT))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return off.QuadPart;
}
zeus::expected<void, std::error_code> FileWrapper::Flush()
{
    assert(!Empty());
    if (!FlushFileBuffers(_impl->handle))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> FileWrapper::CreateTime()
{
    assert(!Empty());
    FILETIME creationTime {};
    if (!GetFileTime(_impl->handle, &creationTime, nullptr, nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return FiletimeToSystemTime(creationTime);
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> FileWrapper::LastAccessTime()
{
    assert(!Empty());
    FILETIME accessTime {};
    if (!GetFileTime(_impl->handle, nullptr, &accessTime, nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return FiletimeToSystemTime(accessTime);
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> FileWrapper::LastWriteTime()
{
    assert(!Empty());
    FILETIME writeTime {};
    if (!GetFileTime(_impl->handle, nullptr, nullptr, &writeTime))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return FiletimeToSystemTime(writeTime);
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> zeus::FileWrapper::LastChangeTime()
{
    assert(!Empty());
    FILE_BASIC_INFO info {};
    if (!GetFileInformationByHandleEx(_impl->handle, FileBasicInfo, &info, sizeof(info)))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return FiletimeToSystemTime(info.ChangeTime);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::Open(const std::filesystem::path& path, OpenMode mode)
{
    return CreateFileWrapper(path, mode, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, false);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::Create(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return CreateFileWrapper(path, mode, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, autoFlush);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::OpenOrCreate(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return CreateFileWrapper(path, mode, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, autoFlush);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::Truncate(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return CreateFileWrapper(path, mode, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, autoFlush);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::OpenSymbolLink(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return CreateFileWrapper(path, mode, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, autoFlush);
}

} // namespace zeus
#endif