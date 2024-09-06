#include "zeus/foundation/resource/file_mapping.h"
#ifdef _WIN32
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/ipc/memory_mapping.h"

namespace zeus
{

struct FileMappingImpl
{
    WinHandle                      mappingHandle;
    std::unique_ptr<MemoryMapping> mapping;
    bool                           writable = false;
    uint64_t                       fileSize = 0;
};

FileMapping::FileMapping() : _impl(std::make_unique<FileMappingImpl>())
{
}

FileMapping::FileMapping(FileMapping&& other) noexcept : _impl(std::move(other._impl))
{
}

FileMapping& FileMapping::operator=(FileMapping&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this; // TODO: 在此处插入 return 语句
}

FileMapping::~FileMapping()
{
}

zeus::expected<void, std::error_code> FileMapping::Map(uint64_t offset, uint64_t length)
{
    _impl->mapping.reset();
    auto mapping = MemoryMapping::Map(_impl->mappingHandle, length, offset, !_impl->writable);
    if (!mapping.has_value())
    {
        return zeus::unexpected(mapping.error());
    }
    _impl->mapping = std::make_unique<MemoryMapping>(std::move(mapping.value()));
    return {};
}

zeus::expected<void, std::error_code> FileMapping::MapAll(uint64_t offset)
{
    _impl->mapping.reset();
    auto mapping = MemoryMapping::Map(_impl->mappingHandle, _impl->fileSize - offset, offset, !_impl->writable);
    if (!mapping.has_value())
    {
        return zeus::unexpected(mapping.error());
    }
    _impl->mapping = std::make_unique<MemoryMapping>(std::move(mapping.value()));
    return {};
}

bool FileMapping::Empty() const
{
    if (!_impl->mapping)
    {
        return true;
    }
    return _impl->mapping->Empty();
}

void* FileMapping::Data() const
{
    if (!_impl->mapping)
    {
        return nullptr;
    }
    return _impl->mapping->Data();
}

uint64_t FileMapping::Size() const
{
    if (!_impl->mapping)
    {
        return 0;
    }
    return _impl->mapping->Size();
}

uint64_t zeus::FileMapping::FileSize() const
{
    return _impl->fileSize;
}

zeus::expected<void, std::error_code> FileMapping::UnMap()
{
    if (!_impl->mapping)
    {
        return {};
    }
    return _impl->mapping->UnMap();
}

zeus::expected<void, std::error_code> FileMapping::Flush()
{
    if (!_impl->mapping)
    {
        return {};
    }
    return _impl->mapping->Flush();
}

zeus::expected<FileMapping, std::error_code> FileMapping::Create(const std::filesystem::path& path, bool writable)
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
    WinHandle handle                   = CreateFileW(
        path.c_str(), (writable ? FILE_GENERIC_WRITE : 0) | FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &securittyAttr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if (!handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return Create(handle, writable);
}

zeus::expected<FileMapping, std::error_code> FileMapping::Create(PlatformFileHandle file, bool writable)
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
    LARGE_INTEGER fileSize             = {};
    if (!GetFileSizeEx(file, &fileSize))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WinHandle mappingHandle =
        CreateFileMappingW(file, &securittyAttr, writable ? PAGE_READWRITE : PAGE_READONLY, fileSize.HighPart, fileSize.LowPart, nullptr);
    if (!mappingHandle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    FileMapping map;
    map._impl->mappingHandle = std::move(mappingHandle);
    map._impl->writable      = writable;
    map._impl->fileSize      = fileSize.QuadPart;
    return map;
}

} // namespace zeus
#endif
