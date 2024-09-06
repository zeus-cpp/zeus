#pragma once
#include <memory>
#include <thread>
#include <string>
#include <functional>
namespace zeus
{
struct AdvancedThreadImpl;
class AdvancedThread
{
public:
    AdvancedThread(const std::string& name, bool automatic = false);
    ~AdvancedThread();
    AdvancedThread(const AdvancedThread&) = delete;
    AdvancedThread(AdvancedThread&& other) noexcept;
    AdvancedThread& operator=(const AdvancedThread&) = delete;
    AdvancedThread& operator=(AdvancedThread&& other) noexcept;
    bool            IsCurrent() const;
    std::thread::id Id() const;
    bool            IsRunning();
    bool            Start();
    void            Stop();
    void            SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback);
    void            Post(const std::function<void()>& task);
    void            Invoke(const std::function<void()>& task);
    template<class ReturnT, typename = typename std::enable_if<!std::is_void<ReturnT>::value>::type>
    ReturnT Invoke(std::function<ReturnT()>&& task)
    {
        ReturnT result;
        Invoke([task, &result] { result = task(); });
        return result;
    }

private:
    std::unique_ptr<AdvancedThreadImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
