#include "zeus/foundation/ipc/shared_memory.h"
#ifdef __linux__
#include <tuple>
#include <filesystem>
#include <cassert>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/crypt/uuid.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/resource/auto_release.h"

namespace zeus
{

namespace
{
const std::string           kMemoryExtension = ".zeus_shared_memory";
const std::filesystem::path kShmDir("/dev/shm");

zeus::expected<std::tuple<LinuxFileDescriptor, uint64_t>, std::error_code> LoadSharedMemory(const std::string& name, bool readOnly)
{
    LinuxFileDescriptor shm = shm_open(
        (name).c_str(), readOnly ? O_RDONLY : O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH
    );
    if (shm.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    struct stat shmStat = {};
    if (fstat(shm.Fd(), &shmStat) != 0)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return std::make_tuple(std::move(shm), shmStat.st_size);
}

zeus::expected<LinuxFileDescriptor, std::error_code> CreateSharedMemory(const std::string& name, uint64_t size, bool readOnly)
{
    LinuxFileDescriptor shm =
        shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
    if (shm.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (ftruncate(shm.Fd(), size) != 0)
    {
        return zeus::unexpected(GetLastSystemError());
    }

    return shm;
}

} // namespace

struct SharedMemoryImpl
{
    std::string         name;
    uint64_t            size = 0;
    LinuxFileDescriptor file;
};

SharedMemory::SharedMemory() : _impl(std::make_unique<SharedMemoryImpl>())
{
}

SharedMemory::SharedMemory(SharedMemory&& other) noexcept : _impl(std::move(other._impl))
{
}

SharedMemory& zeus::SharedMemory::operator=(SharedMemory&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

SharedMemory::~SharedMemory()
{
    if (_impl)
    {
        _impl->file.Close();
    }
}

std::string SharedMemory::Name() const
{
    return _impl->name;
}

size_t SharedMemory::Size() const
{
    return _impl->size;
}

zeus::expected<MemoryMapping, std::error_code> SharedMemory::Map(uint64_t size, uint64_t offset, bool readOnly)
{
    assert(!_impl->file.Empty());
    return MemoryMapping::Map(_impl->file.FileDescriptor(), size, offset, readOnly);
}

int SharedMemory::FileDescriptor() const
{
    return _impl->file.FileDescriptor();
}

zeus::expected<SharedMemory, std::error_code> SharedMemory::OpenOrCreate(const std::string& name, uint64_t size, bool readOnly)
{
    if (auto result = LoadSharedMemory(name, readOnly); result.has_value())
    {
        SharedMemory memory;
        memory._impl->file = std::move(std::get<LinuxFileDescriptor>(result.value()));
        memory._impl->size = std::get<uint64_t>(result.value());
        memory._impl->name = name;
        return std::move(memory);
    }
    auto id           = Uuid::GenerateRandom().toString() + kMemoryExtension;
    auto sharedMemory = CreateSharedMemory(id, size, readOnly);
    if (!sharedMemory.has_value())
    {
        return zeus::unexpected(sharedMemory.error());
    }
    AutoRelease autoUnlink([&id]() { shm_unlink(id.c_str()); });
    const auto  result = link((kShmDir / id).c_str(), (kShmDir / name).c_str());
    if (0 == result)
    {
        SharedMemory memory;
        memory._impl->file = std::move(sharedMemory.value());
        memory._impl->size = size;
        memory._impl->name = name;
        return std::move(memory);
    }
    if (EEXIST != errno)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (auto result = LoadSharedMemory(name, readOnly); result.has_value())
    {
        SharedMemory memory;
        memory._impl->file = std::move(std::get<LinuxFileDescriptor>(result.value()));
        memory._impl->size = std::get<uint64_t>(result.value());
        memory._impl->name = name;
        return std::move(memory);
    }
    else
    {
        return zeus::unexpected(result.error());
    }
}

zeus::expected<SharedMemory, std::error_code> SharedMemory::Open(const std::string& name, bool readOnly)
{
    if (auto result = LoadSharedMemory(name, readOnly); result.has_value())
    {
        SharedMemory memory;
        memory._impl->file = std::move(std::get<LinuxFileDescriptor>(result.value()));
        memory._impl->size = std::get<uint64_t>(result.value());
        memory._impl->name = name;
        return std::move(memory);
    }
    else
    {
        return zeus::unexpected(result.error());
    }
}

zeus::expected<void, std::error_code> SharedMemory::Clear(const std::string& name)
{
    if (0 == shm_unlink((name).c_str()))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

} // namespace zeus
#endif