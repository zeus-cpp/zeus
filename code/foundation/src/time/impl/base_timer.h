#pragma once

#include <memory>
#include <chrono>
#include <string>
#include <functional>
#include <map>
#include <cassert>
#include <atomic>
#include <thread>
#include <mutex>
#include "zeus/foundation/sync/event.h"
#include "base_timer_task.h"

namespace zeus
{
class BaseTimer
{
public:
    BaseTimer(bool automatic);
    virtual ~BaseTimer();
    BaseTimer(const BaseTimer&)            = delete;
    BaseTimer(BaseTimer&&)                 = delete;
    BaseTimer& operator=(const BaseTimer&) = delete;

    size_t AddTimerTask(const std::shared_ptr<BaseTimerTask>& task);
    bool   RemoveTimerTask(size_t id, bool wait);

    virtual std::chrono::nanoseconds::rep Now() const = 0;
    void                                  Start();
    void                                  Stop();
    void                                  SetThreadName(const std::string& name);
    void                                  SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback);
protected:
    virtual void                   Wait(std::chrono::nanoseconds::rep now, std::chrono::nanoseconds::rep end);
    size_t                         GenerateId();
    void                           UpdateQueueTask(const std::shared_ptr<BaseTimerTask>& task, bool notify);
    std::shared_ptr<BaseTimerTask> GetQueueTask(size_t id);
private:
    std::shared_ptr<BaseTimerTask> TopQueueTask();
    void                           Emit(std::shared_ptr<BaseTimerTask>& task, const std::chrono::nanoseconds::rep& now);
    void                           Run();
private:
    std::atomic<bool>                                                            _run = false;
    std::mutex                                                                   _taskMutex;
    std::mutex                                                                   _controlMutex;
    std::map<size_t, std::shared_ptr<BaseTimerTask>>                             _taskMap;
    std::multimap<std::chrono::nanoseconds::rep, std::shared_ptr<BaseTimerTask>> _timeQueue;
    Event                                                                        _event;
    std::thread                                                                  _thread;
    std::string                                                                  _threadName;
    std::atomic<uint64_t>                                                        _threadId;
    std::function<void(const std::exception& exception)>                         _exceptionCallback;
    std::atomic<size_t>                                                          _idGenerator = 0;
    const bool                                                                   _automatic;
};
} // namespace zeus
