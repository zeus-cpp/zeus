#pragma once

namespace zeus
{
//此类用于模版编程中实现无锁版本版本实现
class DummySharedMutex
{
public:
    DummySharedMutex() {}
    ~DummySharedMutex() {}
    DummySharedMutex(const DummySharedMutex&)            = delete;
    DummySharedMutex& operator=(const DummySharedMutex&) = delete;
    DummySharedMutex(DummySharedMutex&&) noexcept {}
    DummySharedMutex& operator=(DummySharedMutex&&) noexcept { return *this; };
    bool              try_lock() { return true; }
    void              lock() {};
    void              unlock() {};

    bool try_lock_shared() { return true; };
    void lock_shared() {};
    void unlock_shared() {};
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
