#include "base_timer.h"
#include "zeus/foundation/thread/thread_utils.h"
namespace zeus
{

BaseTimer::BaseTimer(bool automatic) : _automatic(automatic)
{
}
BaseTimer::~BaseTimer()
{
    if (_automatic)
    {
        Stop();
    }
}
size_t BaseTimer::AddTimerTask(const std::shared_ptr<BaseTimerTask>& task)
{
    {
        std::unique_lock lock(_taskMutex);
        assert(_taskMap.end() == _taskMap.find(task->Id()));
        auto end = task->End();
        task->SetQueuePoint(end);
        _timeQueue.emplace(end, task);
        _taskMap.emplace(task->Id(), task);
    }
    _event.Notify();
    if (_automatic)
    {
        Start();
    }
    return task->Id();
}
bool BaseTimer::RemoveTimerTask(size_t id, bool wait)
{
    std::shared_ptr<BaseTimerTask> task;
    {
        std::unique_lock lock(_taskMutex);
        if (auto iter = _taskMap.find(id); iter != _taskMap.end())
        {
            task       = iter->second;
            auto range = _timeQueue.equal_range(task->QueuePoint());
            for (auto item = range.first; item != range.second; ++item)
            {
                if (task == item->second)
                {
                    _timeQueue.erase(item);
                    break;
                }
            }
            _taskMap.erase(iter);
            return true;
        }
    }

    if (task)
    {
        if (wait)
        {
            task->Disable();
        }
        return true;
    }
    else
    {
        return false;
    }
}

void BaseTimer::Start()
{
    if (_threadId == GetThreadId())
    {
        return;
    }
    std::unique_lock controlLock(_controlMutex);
    if (!_run)
    {
        _run = true;
        _event.Reset();
        _thread = std::thread(&BaseTimer::Run, this);
    }
}
void BaseTimer::Stop()
{
    std::unique_lock controlLock(_controlMutex);
    if (_run)
    {
        _run = false;
        _event.Notify();
        if (_thread.joinable())
        {
            _thread.join();
        }
    }
    {
        std::unique_lock lock(_taskMutex);
        _timeQueue.clear();
        _taskMap.clear();
    }
}
void BaseTimer::SetThreadName(const std::string& name)
{
    _threadName = name;
}
void BaseTimer::SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback)
{
    _exceptionCallback = callback;
}

void BaseTimer::Wait(std::chrono::nanoseconds::rep now, std::chrono::nanoseconds::rep end)
{
    _event.WaitTimeout(std::chrono::steady_clock::duration(end - now));
}
size_t BaseTimer::GenerateId()
{
    return ++_idGenerator;
}

void BaseTimer::UpdateQueueTask(const std::shared_ptr<BaseTimerTask>& task, bool notify)
{
    std::unique_lock lock(_taskMutex);
    auto             range = _timeQueue.equal_range(task->QueuePoint());
    for (auto item = range.first; item != range.second; ++item)
    {
        if (task == item->second)
        {
            _timeQueue.erase(item);
            auto end = task->End();
            task->SetQueuePoint(end);
            _timeQueue.insert(std::make_pair(task->QueuePoint(), task));
            break;
        }
    }
    if (notify)
    {
        _event.Notify();
    }
}
std::shared_ptr<BaseTimerTask> BaseTimer::GetQueueTask(size_t id)
{
    std::unique_lock lock(_taskMutex);
    if (auto iter = _taskMap.find(id); iter != _taskMap.end())
    {
        return iter->second;
    }

    return nullptr;
}
std::shared_ptr<BaseTimerTask> BaseTimer::TopQueueTask()
{
    std::unique_lock lock(_taskMutex);
    if (_timeQueue.empty())
    {
        return nullptr;
    }
    auto iter = _timeQueue.begin();
    return iter->second;
}
void BaseTimer::Emit(std::shared_ptr<BaseTimerTask>& task, const std::chrono::nanoseconds::rep& now)
{
    if (task->Emit(now, _exceptionCallback))
    {
        UpdateQueueTask(task, false);
    }
    else
    {
        RemoveTimerTask(task->Id(), false);
    }
}
void BaseTimer::Run()
{
    _threadId = GetThreadId();
    if (_threadName.empty())
    {
        SetThreadName("Timer");
    }
    else
    {
        SetThreadName(_threadName + "-Timer");
    }
    while (_run)
    {
        auto task = TopQueueTask();
        if (!task)
        {
            //暂时没有定时任务
            do
            {
                const bool wait = _event.WaitTimeout(std::chrono::minutes(1));
                if (wait || !_automatic)
                {
                    break;
                }
                std::unique_lock<std::mutex> controlLock(_controlMutex, std::try_to_lock);
                if (!controlLock)
                {
                    //如果无法拿到控制锁，可能有其他控制行为，继续正常流程
                    break;
                }
                std::unique_lock lock(_taskMutex);
                if (!_timeQueue.empty())
                {
                    break;
                }
                _run = false;
                _thread.detach();
                return;
            }
            while (false);
            continue;
        }
        assert(task);
        auto now = Now();
        auto end = task->End();
        if (now >= end)
        {
            //已经超过时间了,执行任务
            Emit(task, now);
        }
        else
        {
            task.reset();
            Wait(now, end);
            //哪怕等待到了，也要重新从队列中取一次，因为可能有其他任务插入或者更新
        }
    }
    _threadId = 0;
}
} // namespace zeus
