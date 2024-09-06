#pragma once

#include <chrono>
#include <memory>
#include <functional>

namespace zeus
{
struct ConditionVariableImpl;
/*
       此类的实现与条件变量std::condition_variable完全一致，包括虚假唤醒和提前唤醒问题也会存在，但是为了规范用法，等待接口强制使用谓词。
       std::condition_variable需要配合std::mutex使用，此类也一同定义了兼容std::mutex的接口，调用Wait和Notify系列方法前必须对此对象上锁。
       此类的目的主要是为了避免std::condition_variable的接口中对于等待接口受系统时钟修改的问题,所有的接口使用steady_clock的稳定时钟。
*/
class ConditionVariable
{
public:
    ConditionVariable();
    ~ConditionVariable();
    ConditionVariable(const ConditionVariable&)            = delete;
    ConditionVariable(ConditionVariable&&)                 = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    /*
       *Summary: 等待唤醒
             */
    void Wait(const std::function<bool()>& predicate);

    /*
       *Summary: 超时等待唤醒
          *Parameters:
             *     timeout：超时的时间
                *Return :如果被唤醒，返回true，超时返回false
                   */
    bool WaitTimeout(const std::chrono::steady_clock::duration& duration, const std::function<bool()>& predicate);

    /*
       *Summary: 超时等待唤醒
          *Parameters:
             *     point：指定的时间
                *Return :如果被唤醒，返回true，超时返回false
                   */
    bool WaitUntil(const std::chrono::steady_clock::time_point& point, const std::function<bool()>& predicate);

    /*
       *Summary: 唤醒一个等待的线程
             */
    void NotifyOne();

    /*
       *Summary: 唤醒所有等待的线程
             */
    void NotifyAll();

    void Lock();
    void Unlock();
    bool TryLock();

    //以下为兼容std::mutex的接口
    void lock();
    void unlock();
    bool try_lock();

private:
    std::unique_ptr<ConditionVariableImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
