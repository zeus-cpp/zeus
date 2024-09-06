#pragma once
#include <chrono>
#include <memory>
#include <mutex>
#include <functional>
namespace zeus
{
class BaseTimerTask
{
public:
    BaseTimerTask(size_t id);
    virtual ~BaseTimerTask();
    BaseTimerTask(const BaseTimerTask&)            = delete;
    BaseTimerTask(BaseTimerTask&&)                 = delete;
    BaseTimerTask& operator=(const BaseTimerTask&) = delete;
    size_t         Id() const;
    void           Disable();
    void           lock();
    void           unlock();
    virtual bool   Emit(const std::chrono::nanoseconds::rep& now, const std::function<void(const std::exception& exception)>& exceptionCallback) = 0;
    virtual std::chrono::nanoseconds::rep End()                                                                                                  = 0;
    std::chrono::nanoseconds::rep         QueuePoint() const;
    void                                  SetQueuePoint(const std::chrono::nanoseconds::rep& point);
protected:
    bool         IsEnable() const;
    virtual void DisableImpl();
private:
    size_t                        _id     = 0;
    bool                          _enable = true;
    std::recursive_mutex          _mutex;
    std::chrono::nanoseconds::rep _queuePoint = 0;
};
} // namespace zeus