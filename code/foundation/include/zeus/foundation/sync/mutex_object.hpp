#pragma once
#include <mutex>
#include <utility>
#include <type_traits>

namespace zeus
{
template<typename T, typename MutexType = std::mutex>
class MutexObject
{
public:
    using UseMutexType = MutexType;
    MutexObject() {}
    template<typename... Args>
    MutexObject(Args&&... args) : _obj(std::forward<Args>(args)...)
    {
    }

    MutexObject(const T& obj) : _obj(obj) {}

    MutexObject(T&& obj) : _obj(std::forward<T>(obj)) {}

    T* operator->() { return &_obj; }

    T& operator*() { return _obj; }

    MutexObject& operator=(const T& obj)
    {
        _obj = obj;
        return *this;
    }

    MutexObject& operator=(T&& obj)
    {
        _obj = std::forward<T>(obj);
        return *this;
    }

    operator T&() { return _obj; }

    MutexType& Mutex() { return _mutex; }
    T&         Value() { return _obj; }

    T Load()
    {
        std::lock_guard<MutexType> lock(_mutex);
        return _obj;
    }

    void Store(const T& obj)
    {
        std::lock_guard<MutexType> lock(_mutex);
        _obj = obj;
    }

    void Store(T&& obj)
    {
        std::lock_guard<MutexType> lock(_mutex);
        _obj = std::forward<T>(obj);
    }

    void Swap(T& value)
    {
        std::lock_guard<MutexType> lock(_mutex);
        using std::swap;
        swap(_obj, value);
    }
    //兼容标准库锁接口
    void lock() { _mutex.lock(); }
    bool try_lock() { return _mutex.try_lock(); }
    void unlock() { _mutex.unlock(); }

private:
    T         _obj;
    MutexType _mutex;
};

} // namespace zeus
// NOLINTBEGIN(cppcoreguidelines-macro-usage,cert-dcl37-c,cert-dcl51-cpp) 没办法，要定义变量，只能用宏
#define _ZEUS_TEMP_LOCK_NAME(line) lock_##line

#define ZEUS_TEMP_LOCK_NAME(line) _ZEUS_TEMP_LOCK_NAME(line)

#define MUTEX_OBJECT_LOCK(mutexObject) \
    std::lock_guard<typename std::remove_reference<decltype((mutexObject))>::type::UseMutexType> ZEUS_TEMP_LOCK_NAME(__LINE__)((mutexObject).Mutex());
// NOLINTEND(cppcoreguidelines-macro-usage,cert-dcl37-c,cert-dcl51-cpp)
#include "zeus/foundation/core/zeus_compatible.h"
