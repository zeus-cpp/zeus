#include "zeus/foundation/resource/file_mapping.h"

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/ipc/memory_mapping.h"

namespace zeus
{

struct FileMappingImpl
{
    LinuxFileDescriptor            mappingFileDescriptor;
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
    return *this;
}

FileMapping::~FileMapping()
{
}

zeus::expected<void, std::error_code> FileMapping::Map(uint64_t offset, uint64_t length)
{
    _impl->mapping.reset();
    auto mapping = MemoryMapping::Map(_impl->mappingFileDescriptor.FileDescriptor(), length, offset, !_impl->writable);
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
    auto mapping = MemoryMapping::Map(_impl->mappingFileDescriptor.FileDescriptor(), _impl->fileSize - offset, offset, !_impl->writable);
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
    LinuxFileDescriptor fileDescriptor = open(path.c_str(), writable ? O_RDWR : O_RDONLY);
    if (fileDescriptor.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return Create(fileDescriptor.FileDescriptor(), writable);
}

zeus::expected<FileMapping, std::error_code> FileMapping::Create(PlatformFileHandle file, bool writable)
{
    struct stat statbuf;
    if (fstat(file, &statbuf) == -1)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    LinuxFileDescriptor mappingFileDescriptor = dup(file);
    if (mappingFileDescriptor.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    FileMapping map;
    map._impl->mappingFileDescriptor = std::move(mappingFileDescriptor);
    map._impl->writable              = writable;
    map._impl->fileSize              = statbuf.st_size;
    return std::move(map);
}
} // namespace zeus
#endif
