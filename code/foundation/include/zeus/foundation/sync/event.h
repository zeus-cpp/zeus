#pragma once

#include <chrono>
#include <memory>

namespace zeus
{
struct EventImpl;

class Event
{
public:
    Event();
    ~Event();
    Event(const Event&)            = delete;
    Event(Event&&)                 = delete;
    Event& operator=(const Event&) = delete;
    /*

    *Summary: 重置状态
    *Info：重置为未激发状态,使用全部唤醒的时间重复使用前一定需要调用此接口。

    */
    void   Reset();

    /*

    *Summary: 等待事件被激发
    *Info：激发后会自动重置状态，可被再次激发。

    */
    void Wait();

    /*

    *Summary: 超时等待事件被激发
    *Parameters:
    *     timeout：超时的时间

    *Return :如果被正确激发，返回true，超时返回false

    */

    bool WaitTimeout(const std::chrono::steady_clock::duration& duration);

    /*

    *Summary: 超时等待事件被激发
    *Parameters:
    *     point：指定的时间

    *Return :如果被正确激发，返回true，超时返回false

    */

    bool WaitUntil(const std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration>& point);

    /*

    *Summary: 激发事件

    *Return :

    */
    void Notify();

    /*

    *Summary: 激发事件,通知所有在等待的对象,并且激发状态一直存在，直到调用了Reset接口

    *Return :

    */
    void NotifyAll();

private:
    std::unique_ptr<EventImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
