#pragma once

#include <functional>
#include <memory>
#include <thread>
#include <future>

namespace zeus
{
struct ThreadPoolImpl;
class ThreadPool
{
public:
    using Task = std::function<void()>;
    ThreadPool(size_t coreSize = 2, bool autoExpansion = false, size_t maxSize = std::thread::hardware_concurrency(), bool automatic = true);
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&& other) noexcept;
    ThreadPool& operator=(const ThreadPool) = delete;
    ThreadPool& operator=(ThreadPool&& other) noexcept;

    void SetName(const std::string& name);

    void SetTaskBlockQueueSize(size_t size);

    //如果不需要自己捕捉异常，建议使用CommitTask，内部会自动捕捉异常并记录
    void CommitTask(const Task& task);

    void CommitTask(Task&& task);

    //如果需要自己捕捉异常，建议使用Commit，可以通过返回的future获取异常
    template<typename F, typename... Args>
    auto Commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        using ResType = decltype(f(args...)); // 函数f的返回值类型
        auto task     = std::make_shared<std::packaged_task<ResType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto c        = [task]()
        {
            (*task)();
        };
        CommitTask(c);
        std::future<ResType> future = task->get_future();
        return future;
    }
    void Stop();
    void Start();

    void SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback);
    // 判断线程是否在线程池中，此函数应该仅用于调试目的。此函数请尽量不能在shutdown、stop、等停止函数运行时调用
    bool IsPoolThread(std::thread::id id = std::this_thread::get_id());
private:
    std::unique_ptr<ThreadPoolImpl> _impl;
};
using ThreadPoolUtils = ThreadPool;
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
