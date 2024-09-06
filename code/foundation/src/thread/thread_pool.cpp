#include "zeus/foundation/thread/thread_pool.h"
#include <thread>
#include <algorithm>
#include <optional>
#include <future>
#include <list>
#include <atomic>
#include <queue>
#include <set>
#include <vector>
#include <memory>
#include <mutex>
#include <utility>
#include <cassert>
#include "zeus/foundation/sync/mutex_object.hpp"
#include "zeus/foundation/sync/event.h"
#include "zeus/foundation/thread/thread_utils.h"

namespace zeus
{

struct ThreadPoolImpl
{
    std::list<std::thread>                               pool;
    MutexObject<std::set<uint64_t>>                      threadIds;
    std::queue<ThreadPool::Task>                         tasks;
    std::mutex                                           controlMutex;
    std::mutex                                           taskMutex;
    std::atomic<bool>                                    run {false};
    Event                                                event;
    size_t                                               coreSize      = 0;
    size_t                                               maxSize       = 0;
    bool                                                 autoExpansion = false;
    bool                                                 automatic     = false;
    std::string                                          threadName;
    std::atomic_size_t                                   taskBlockQueueSize = 1;
    std::function<void(const std::exception& exception)> exceptionCallback;
};

namespace
{

std::optional<ThreadPool::Task> GetRunnableTask(ThreadPoolImpl& impl)
{
    std::lock_guard<std::mutex> lock(impl.taskMutex);
    if (impl.tasks.empty())
    {
        return std::nullopt;
    }
    auto task = std::move(impl.tasks.front());
    impl.tasks.pop();
    assert(task);
    return std::make_optional<ThreadPool::Task>(std::move(task));
}

void Run(ThreadPoolImpl& impl, bool core)
{
    {
        std::unique_lock lock(impl.threadIds);
        impl.threadIds->emplace(GetThreadId());
    }
    if (impl.threadName.empty())
    {
#ifdef _WIN32
        SetThreadName(std::string("ThreadPool-") + (core ? "core" : "temp"));
#else
        SetThreadName(std::string("TP-") + (core ? "core" : "temp"));
#endif
    }
    else
    {
#ifdef _WIN32
        SetThreadName(std::string("ThreadPool[") + impl.threadName + "]" + (core ? "core" : "temp"));
#else
        SetThreadName(std::string("TP[") + impl.threadName + "]" + (core ? "core" : "temp"));
#endif
    }
    while (impl.run)
    {
        auto task = GetRunnableTask(impl);
        if (!task.has_value())
        {
            if (core)
            {
                impl.event.Wait();
                continue;
            }
            else
            {
                if (impl.event.WaitTimeout(std::chrono::minutes(1)))
                {
                    continue;
                }
            }
            std::unique_lock<std::mutex> controlLock(impl.controlMutex, std::try_to_lock);
            if (!controlLock)
            {
                continue;
            }
            {
                std::lock_guard<std::mutex> lock(impl.taskMutex);
                if (!impl.tasks.empty())
                {
                    continue;
                }
            }
            for (auto iter = impl.pool.begin(); iter != impl.pool.end(); ++iter)
            {
                if (iter->get_id() == std::this_thread::get_id())
                {
                    iter->detach();
                    impl.pool.erase(iter);
                    break;
                }
            }
            return;
        }
        assert(task.has_value());
        assert(task.value());
        try
        {
            task.value()();
        }
        catch (const std::exception& e)
        {
            if (impl.exceptionCallback)
            {
                impl.exceptionCallback(e);
            }
        }
    }
    {
        std::unique_lock lock(impl.threadIds);
        impl.threadIds->erase(GetThreadId());
    }
}
} // namespace

ThreadPool::ThreadPool(size_t coreSize, bool autoExpansion, size_t maxSize, bool automatic) : _impl(std::make_unique<ThreadPoolImpl>())
{
    _impl->coreSize      = coreSize;
    _impl->maxSize       = maxSize;
    _impl->autoExpansion = autoExpansion;
    _impl->automatic     = automatic;
    assert(coreSize || autoExpansion);
}
ThreadPool::~ThreadPool()
{
    if (!_impl)
    {
        return;
    }
    if (_impl->automatic)
    {
        Stop();
    }
}
ThreadPool::ThreadPool(ThreadPool&& other) noexcept : _impl(std::move(other._impl))
{
}

ThreadPool& ThreadPool::operator=(ThreadPool&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
void ThreadPool::SetName(const std::string& name)
{
    _impl->threadName = name;
}
void ThreadPool::SetTaskBlockQueueSize(size_t size)
{
    _impl->taskBlockQueueSize = size;
}

void ThreadPool::CommitTask(const Task& task)
{
    CommitTask(Task(task));
}
void ThreadPool::CommitTask(Task&& task)
{
    assert(task);
    if (!task)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(_impl->taskMutex);
        _impl->tasks.emplace(std::move(task));
    }
    _impl->event.Notify();
    {
        std::unique_lock lock(_impl->threadIds);
        if (_impl->threadIds->count(GetThreadId()))
        {
            return;
        }
    }
    {
        std::lock_guard<std::mutex> lock(_impl->controlMutex);
        if (!_impl->automatic && !_impl->run)
        {
            return;
        }
        if (!_impl->run)
        {
            _impl->run = true;
            for (size_t i = 0; i < _impl->coreSize; ++i)
            {
                _impl->pool.emplace_back(std::bind(&Run, std::ref(*_impl), true));
            }
        }
        if (_impl->autoExpansion)
        {
            assert(_impl->pool.size() >= _impl->coreSize);
            if (_impl->pool.size() >= _impl->maxSize)
            {
                return;
            }
            if (_impl->tasks.size() <= _impl->taskBlockQueueSize && !_impl->pool.empty())
            {
                return;
            }
            _impl->pool.emplace_back(std::bind(&Run, std::ref(*_impl), false));
        }
    }
}
void ThreadPool::Stop()
{
    std::lock_guard<std::mutex> controlLock(_impl->controlMutex);
    if (_impl->run)
    {
        _impl->run = false;
        _impl->event.NotifyAll();
        for (auto& thread : _impl->pool)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        _impl->pool.clear();
        _impl->event.Reset();
    }
    {
        std::unique_lock lock(_impl->taskMutex);
        std::queue<Task>().swap(_impl->tasks);
    }
}
void ThreadPool::Start()
{
    {
        std::unique_lock lock(_impl->threadIds);
        if (_impl->threadIds->count(GetThreadId()))
        {
            return;
        }
    }
    std::lock_guard<std::mutex> lock(_impl->controlMutex);
    if (!_impl->run)
    {
        _impl->run = true;
        for (size_t i = 0; i < _impl->coreSize; ++i)
        {
            _impl->pool.emplace_back(std::bind(&Run, std::ref(*_impl), true));
        }
    }
}

void ThreadPool::SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback)
{
    _impl->exceptionCallback = callback;
}
bool ThreadPool::IsPoolThread(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(_impl->controlMutex);
    return std::any_of(_impl->pool.begin(), _impl->pool.end(), [id](const std::thread& thread) { return thread.get_id() == id; });
}
} // namespace zeus
