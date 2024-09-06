#include "zeus/foundation/ipc/global_mutex.h"
#ifdef __linux__
#include <filesystem>
#include <cassert>
#include <unistd.h>
#include <pthread.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/crypt/uuid.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/ipc/memory_mapping.h"
#include "zeus/foundation/ipc/shared_memory.h"

namespace zeus
{

struct GlobalMutexImpl
{
    std::string                    name;
    std::unique_ptr<MemoryMapping> mapping;
    pthread_mutex_t*               mutex = nullptr;
#ifndef _NDEBUG
    bool isOwner = false;
#endif
};

namespace
{
const std::string           kMutexExtension = ".zeus_global_mutex_1";
const std::filesystem::path kShmDir("/dev/shm");

zeus::expected<MemoryMapping, std::error_code> LoadSharedPThreadMutex(const std::string& name)
{
    auto memory = SharedMemory::Open(name, false);
    if (!memory.has_value())
    {
        return zeus::unexpected(memory.error());
    }
    auto mapping = memory->Map(sizeof(pthread_mutex_t));
    if (!mapping.has_value())
    {
        return zeus::unexpected(mapping.error());
    }
    return std::move(mapping);
}

zeus::expected<MemoryMapping, std::error_code> CreateSharedPThreadMutex(const std::string& name)
{
    auto memory = SharedMemory::OpenOrCreate(name, sizeof(pthread_mutex_t), false);
    if (!memory.has_value())
    {
        return zeus::unexpected(memory.error());
    }
    auto mapping = memory->Map(sizeof(pthread_mutex_t));
    if (!mapping.has_value())
    {
        return zeus::unexpected(mapping.error());
    }
    pthread_mutex_t* mutex   = static_cast<pthread_mutex_t*>(mapping->Data());
    *(mutex)                 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutexattr_t attr = {};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    if (pthread_mutex_init(mutex, &attr) != 0)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return std::move(mapping);
}
} // namespace

GlobalMutex::GlobalMutex() : _impl(std::make_unique<GlobalMutexImpl>())
{
}

GlobalMutex::GlobalMutex(GlobalMutex&& other) noexcept : _impl(std::move(other._impl))
{
}

GlobalMutex& GlobalMutex::operator=(GlobalMutex&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

GlobalMutex::~GlobalMutex()
{
    if (_impl)
    {
#ifndef _NDEBUG
        assert(!_impl->isOwner);
#endif
        _impl->mutex = nullptr;
    }
}

std::string GlobalMutex::Name() const
{
    return _impl->name;
}

zeus::expected<void, std::error_code> GlobalMutex::Lock()
{
    assert(_impl->mapping->Data());
    assert(_impl->mutex);
    do
    {
        if (const int result = pthread_mutex_lock(_impl->mutex); 0 == result)
        {
#ifndef _NDEBUG
            _impl->isOwner = true;
#endif
            return {};
        }
        else if (EOWNERDEAD == result)
        {
            if (0 == pthread_mutex_consistent(_impl->mutex))
            {
#ifndef _NDEBUG
                _impl->isOwner = true;
#endif
                return {};
            }
            else
            {
                continue;
            }
        }
        else
        {
            return zeus::unexpected(make_error_code(SystemError {result}));
        }
    }
    while (true);
}

zeus::expected<void, std::error_code> GlobalMutex::Unlock()
{
    assert(_impl->mapping->Data());
    assert(_impl->mutex);
#ifndef _NDEBUG
    assert(_impl->isOwner);
    _impl->isOwner = false;
#endif
    if (const int result = pthread_mutex_unlock(_impl->mutex); 0 == result)
    {
        return {};
    }
    else
    {
        return zeus::unexpected(make_error_code(SystemError {result}));
    }
}

zeus::expected<bool, std::error_code> GlobalMutex::TryLock()
{
    assert(_impl->mapping->Data());
    assert(_impl->mutex);
    const int result = pthread_mutex_trylock(_impl->mutex);
    if (0 == result)
    {
#ifndef _NDEBUG
        _impl->isOwner = true;
#endif
        return true;
    }
    else if (result == EBUSY)
    {
        return false;
    }
    else if (EOWNERDEAD == result)
    {
        if (0 == pthread_mutex_consistent(_impl->mutex))
        {
#ifndef _NDEBUG
            _impl->isOwner = true;
#endif
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return zeus::unexpected(make_error_code(SystemError {result}));
    }
}

zeus::expected<GlobalMutex, std::error_code> GlobalMutex::OpenOrCreate(const std::string& name)
{
    if (auto result = LoadSharedPThreadMutex(name + kMutexExtension); result.has_value())
    {
        GlobalMutex mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(result.value()));
        mutex._impl->mutex   = static_cast<pthread_mutex_t*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    auto id          = Uuid::GenerateRandom().toString() + kMutexExtension;
    auto sharedMutex = CreateSharedPThreadMutex(id);
    if (!sharedMutex.has_value())
    {
        return zeus::unexpected(sharedMutex.error());
    }
    AutoRelease autoUnlink([&id]() { SharedMemory::Clear(id); });
    const auto  result = link((kShmDir / id).c_str(), (kShmDir / (name + kMutexExtension)).c_str());
    if (0 == result)
    {
        GlobalMutex mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(sharedMutex.value()));
        mutex._impl->mutex   = static_cast<pthread_mutex_t*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    if (EEXIST != errno)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (auto result = LoadSharedPThreadMutex(name + kMutexExtension); result.has_value())
    {
        GlobalMutex mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(result.value()));
        mutex._impl->mutex   = static_cast<pthread_mutex_t*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(result.error());
    }
}

zeus::expected<GlobalMutex, std::error_code> GlobalMutex::Open(const std::string& name)
{
    if (auto result = LoadSharedPThreadMutex(name + kMutexExtension); result.has_value())
    {
        GlobalMutex mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(result.value()));
        mutex._impl->mutex   = static_cast<pthread_mutex_t*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(result.error());
    }
}

zeus::expected<void, std::error_code> zeus::GlobalMutex::Clear(const std::string& name)
{
    return SharedMemory::Clear(name + kMutexExtension);
}
} // namespace zeus
#endif