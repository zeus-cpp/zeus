#pragma once

namespace zeus
{
//此类用于模版编程中实现无锁版本版本实现
class DummyMutex
{
public:
    DummyMutex() {}
    ~DummyMutex() {}
    DummyMutex(const DummyMutex&)            = delete;
    DummyMutex& operator=(const DummyMutex&) = delete;
    DummyMutex(DummyMutex&&) noexcept {}
    DummyMutex& operator=(DummyMutex&&) noexcept { return *this; };
    bool        try_lock() { return true; }
    void        lock() {};
    void        unlock() {};
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
