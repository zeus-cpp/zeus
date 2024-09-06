#include "zeus/foundation/ipc/global_event.h"
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
#include "zeus/foundation/time/time_utils.h"

namespace zeus
{

struct SharedData
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            state       = false;
    bool            manual      = false;
    uint64_t        pid         = 0;
    uint64_t        processTick = 0;
} __attribute__((aligned(8)));

struct GlobalEventImpl
{
    std::string                    name;
    std::unique_ptr<MemoryMapping> mapping;
    SharedData*                    data = nullptr;
};

namespace
{
const std::string           kEventExtension = ".zeus_global_event_1";
const std::filesystem::path kShmDir("/dev/shm");

class AutoLock
{
public:
    AutoLock(pthread_mutex_t& mutex) : _mutex(mutex) {}
    AutoLock(const AutoLock&)            = delete;
    AutoLock(AutoLock&&)                 = delete;
    AutoLock& operator=(const AutoLock&) = delete;
    AutoLock& operator=(AutoLock&&)      = delete;

    ~AutoLock() { pthread_mutex_unlock(&_mutex); }
private:
    pthread_mutex_t& _mutex;
};

zeus::expected<void, std::error_code> LockMutex(pthread_mutex_t& mutex)
{
    do
    {
        if (const int result = pthread_mutex_lock(&mutex); 0 == result)
        {
            break;
        }
        else if (EOWNERDEAD == result)
        {
            if (0 == pthread_mutex_consistent(&mutex))
            {
                break;
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
    return {};
}

zeus::expected<MemoryMapping, std::error_code> LoadSharedPData(const std::string& name)
{
    auto memory = SharedMemory::Open(name, false);
    if (!memory.has_value())
    {
        return zeus::unexpected(memory.error());
    }
    auto mapping = memory->Map(sizeof(SharedData));
    if (!mapping.has_value())
    {
        return zeus::unexpected(mapping.error());
    }
    return std::move(mapping);
}

zeus::expected<MemoryMapping, std::error_code> CreateSharedPData(const std::string& name, bool manual)
{
    auto memory = SharedMemory::OpenOrCreate(name, sizeof(SharedData), false);
    if (!memory.has_value())
    {
        return zeus::unexpected(memory.error());
    }
    auto mapping = memory->Map(sizeof(SharedData));
    if (!mapping.has_value())
    {
        return zeus::unexpected(mapping.error());
    }
    pthread_mutex_t* mutex        = &static_cast<SharedData*>(mapping->Data())->mutex;
    *(mutex)                      = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutexattr_t mutexAttr = {};
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&mutexAttr, PTHREAD_MUTEX_ROBUST);
    if (pthread_mutex_init(mutex, &mutexAttr) != 0)
    {
        return zeus::unexpected(GetLastSystemError());
    }

    pthread_cond_t* cond        = &static_cast<SharedData*>(mapping->Data())->cond;
    *(cond)                     = PTHREAD_COND_INITIALIZER;
    pthread_condattr_t condAttr = {};
    pthread_condattr_init(&condAttr);
    pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    if (pthread_cond_init(cond, &condAttr) != 0)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    auto creator                                           = Process::GetCurrentProcess();
    static_cast<SharedData*>(mapping->Data())->state       = false;
    static_cast<SharedData*>(mapping->Data())->manual      = manual;
    static_cast<SharedData*>(mapping->Data())->pid         = creator.Id();
    static_cast<SharedData*>(mapping->Data())->processTick = creator.CreateTimeTick();
    return std::move(mapping);
}
} // namespace

GlobalEvent::GlobalEvent() : _impl(std::make_unique<GlobalEventImpl>())
{
}

GlobalEvent::GlobalEvent(GlobalEvent&& other) noexcept : _impl(std::move(other._impl))
{
}

GlobalEvent& GlobalEvent::operator=(GlobalEvent&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

GlobalEvent::~GlobalEvent()
{
    if (_impl)
    {
        _impl->data = nullptr;
    }
}

std::string GlobalEvent::Name() const
{
    return _impl->name;
}

zeus::expected<void, std::error_code> GlobalEvent::Set()
{
    auto lock = LockMutex(_impl->data->mutex);
    if (!lock.has_value())
    {
        return zeus::unexpected(lock.error());
    }
    AutoLock autoLock(_impl->data->mutex);
    _impl->data->state = true;
    if (const int result = pthread_cond_broadcast(&_impl->data->cond); 0 != result)
    {
        return zeus::unexpected(make_error_code(SystemError {result}));
    }
    return {};
}

zeus::expected<void, std::error_code> GlobalEvent::Reset()
{
    auto lock = LockMutex(_impl->data->mutex);
    if (!lock.has_value())
    {
        return zeus::unexpected(lock.error());
    }
    AutoLock autoLock(_impl->data->mutex);
    _impl->data->state = false;
    return {};
}

zeus::expected<void, std::error_code> GlobalEvent::Wait()
{
    auto lock = LockMutex(_impl->data->mutex);
    if (!lock.has_value())
    {
        return zeus::unexpected(lock.error());
    }
    AutoLock autoLock(_impl->data->mutex);
    while (!_impl->data->state)
    {
        if (const int result = pthread_cond_wait(&_impl->data->cond, &_impl->data->mutex); 0 != result)
        {
            return zeus::unexpected(make_error_code(SystemError {result}));
        }
    }
    if (!_impl->data->manual)
    {
        _impl->data->state = false;
    }
    return {};
}

zeus::expected<bool, std::error_code> GlobalEvent::WaitTimeout(const std::chrono::steady_clock::duration& duration)
{
    auto begin        = std::chrono::steady_clock::now();
    auto waitDuration = duration;
    auto lock         = LockMutex(_impl->data->mutex);
    if (!lock.has_value())
    {
        return zeus::unexpected(lock.error());
    }
    AutoLock autoLock(_impl->data->mutex);
    while (!_impl->data->state)
    {
        timespec time = {};
        clock_gettime(CLOCK_MONOTONIC, &time);
        time              = TimeSpecAdd(time, waitDuration);
        const auto result = pthread_cond_timedwait(&_impl->data->cond, &_impl->data->mutex, &time);
        if (0 != result)
        {
            if (ETIMEDOUT == result)
            {
                return false;
            }
            else
            {
                return zeus::unexpected(make_error_code(SystemError {result}));
            }
        }
        auto now = std::chrono::steady_clock::now();
        if (now - begin >= duration)
        {
            return false;
        }
        else
        {
            waitDuration = begin + duration - now;
        }
    }
    if (!_impl->data->manual)
    {
        _impl->data->state = false;
    }
    return true;
}

zeus::expected<zeus::Process::PID, std::error_code> GlobalEvent::AliveCreator()
{
    auto tick = Process::GetProcessCreateTimeTick(_impl->data->pid);
    if (!tick.has_value())
    {
        return zeus::unexpected(tick.error());
    }
    if (tick.value() == _impl->data->processTick)
    {
        return _impl->data->pid;
    }
    else
    {
        return zeus::unexpected(TranslateToSystemError(ESRCH));
    }
}

zeus::expected<GlobalEvent, std::error_code> GlobalEvent::OpenOrCreate(const std::string& name, bool manual)
{
    if (auto result = LoadSharedPData(name + kEventExtension); result.has_value())
    {
        GlobalEvent mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(result.value()));
        mutex._impl->data    = static_cast<SharedData*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    auto id          = Uuid::GenerateRandom().toString() + kEventExtension;
    auto sharedMutex = CreateSharedPData(id, manual);
    if (!sharedMutex.has_value())
    {
        return zeus::unexpected(sharedMutex.error());
    }
    AutoRelease autoUnlink([&id]() { SharedMemory::Clear(id); });
    const auto  result = link((kShmDir / id).c_str(), (kShmDir / (name + kEventExtension)).c_str());
    if (0 == result)
    {
        GlobalEvent mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(sharedMutex.value()));
        mutex._impl->data    = static_cast<SharedData*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    if (EEXIST != errno)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (auto result = LoadSharedPData(name + kEventExtension); result.has_value())
    {
        GlobalEvent mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(result.value()));
        mutex._impl->data    = static_cast<SharedData*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(result.error());
    }
}

zeus::expected<GlobalEvent, std::error_code> GlobalEvent::Open(const std::string& name)
{
    if (auto result = LoadSharedPData(name + kEventExtension); result.has_value())
    {
        GlobalEvent mutex;
        mutex._impl->mapping = std::make_unique<MemoryMapping>(std::move(result.value()));
        mutex._impl->data    = static_cast<SharedData*>(mutex._impl->mapping->Data());
        mutex._impl->name    = name;
        return std::move(mutex);
    }
    else
    {
        return zeus::unexpected(result.error());
    }
}

zeus::expected<void, std::error_code> zeus::GlobalEvent::Clear(const std::string& name)
{
    return SharedMemory::Clear(name + kEventExtension);
}
} // namespace zeus
#endif