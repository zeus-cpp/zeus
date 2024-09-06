#include "zeus/foundation/thread/advanced_thread.h"
#include "zeus/foundation/thread/thread_utils.h"
#include <list>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

namespace zeus
{

struct AdvancedThreadImpl
{
    std::function<void(const std::exception& exception)> exceptionCallback;
    std::recursive_mutex                                 mutex;
    std::mutex                                           taskMutex;
    std::condition_variable          taskCondition; //这里没有使用wait_for，不涉及时间操作，所以可以使用标准库的条件变量
    std::thread                      thread;
    std::atomic<uint64_t>            threadId;
    std::list<std::function<void()>> taskQueue;
    std::atomic<bool>                run       = {false};
    bool                             automatic = false;
    std::string                      name;
};
namespace
{

void Run(AdvancedThreadImpl& impl)
{
    impl.threadId = GetThreadId();
    if (!impl.name.empty())
    {
        SetThreadName(impl.name);
    }
    while (impl.run)
    {
        std::list<std::function<void()>> tasks;
        {
            std::unique_lock<std::mutex> lock(impl.taskMutex);
            impl.taskCondition.wait(lock, [&impl]() { return !impl.run || !impl.taskQueue.empty(); });
            while (!impl.taskQueue.empty())
            {
                tasks.emplace_back(std::move(impl.taskQueue.front()));
                impl.taskQueue.pop_front();
            }
        }
        for (const auto& task : tasks)
        {
            if (task)
            {
                try
                {
                    task();
                }
                catch (std::exception& e)
                {
                    if (impl.exceptionCallback)
                    {
                        impl.exceptionCallback(e);
                    }
                }
            }
        }
    }
    {
        std::unique_lock<std::mutex> lock(impl.taskMutex);
        impl.taskQueue.clear();
    }
    impl.threadId = 0;
}

void PostTask(AdvancedThreadImpl& impl, const std::function<void()>& task)
{
    std::unique_lock<std::mutex> lock(impl.taskMutex);
    impl.taskQueue.emplace_back(task);
    impl.taskCondition.notify_one();
}
} // namespace

AdvancedThread::AdvancedThread(const std::string& name, bool automatic) : _impl(std::make_unique<AdvancedThreadImpl>())
{
    _impl->name      = name;
    _impl->automatic = automatic;
}
AdvancedThread::~AdvancedThread()
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
AdvancedThread::AdvancedThread(AdvancedThread&& other) noexcept : _impl(std::move(other._impl))
{
}
AdvancedThread& AdvancedThread::operator=(AdvancedThread&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
bool AdvancedThread::IsCurrent() const
{
    return std::this_thread::get_id() == Id();
}
std::thread::id AdvancedThread::Id() const
{
    return _impl->thread.get_id();
}

bool AdvancedThread::IsRunning()
{
    return _impl->thread.joinable();
}

bool AdvancedThread::Start()
{
    if (_impl->threadId == GetThreadId())
    {
        return true;
    }
    std::unique_lock<std::recursive_mutex> lock(_impl->mutex);
    if (!_impl->run)
    {
        _impl->run    = true;
        _impl->thread = std::thread(std::bind(&Run, std::ref(*_impl)));
        return true;
    }
    else
    {
        return false;
    }
}
void AdvancedThread::Stop()
{
    std::unique_lock<std::recursive_mutex> lock(_impl->mutex);
    _impl->run = false;
    _impl->taskCondition.notify_one();
    if (_impl->thread.joinable())
    {
        _impl->thread.join();
    }
    {
        std::unique_lock<std::mutex> taskLock(_impl->taskMutex);
        _impl->taskQueue.clear();
    }
}
void AdvancedThread::SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback)
{
    _impl->exceptionCallback = callback;
}
void AdvancedThread::Post(const std::function<void()>& task)
{
    PostTask(*_impl, task);
    if (_impl->automatic)
    {
        Start();
    }
}

void AdvancedThread::Invoke(const std::function<void()>& task)
{
    {
        std::unique_lock<std::recursive_mutex> lock(_impl->mutex);
        if (std::this_thread::get_id() == _impl->thread.get_id())
        {
            task();
            return;
        }
    }

    std::promise<void> promise;
    auto               future = promise.get_future();
    Post(
        [&task, &promise]()
        {
            task();
            promise.set_value();
        }
    );
    future.get();
}

} // namespace zeus