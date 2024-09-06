#pragma once
#include <memory>
#include <mutex>
#include <type_traits>
#include <cstring>

namespace zeus
{
template<typename Value, typename Mutex = std::mutex>
class FixedBufferQueue
{
public:
    using ValueType = Value;
    FixedBufferQueue(size_t capacity) : _current(0), _capacity(capacity)
    {
        static_assert(std::is_trivial<Value>::value, "Must is POD");
        static_assert(std::is_standard_layout<Value>::value, "Must is POD");
        _data = std::make_unique<Value[]>(capacity);
    }
    ~FixedBufferQueue() {}
    //count表示指针中元素的个数，不是字节长度，truncation表示当容量已满时是否允许阶段，返回的是实际入队的元素个数
    size_t Push(const Value* data, size_t count, bool truncation = false)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto                   expectCapacity = _current + count;
        size_t                 pushCount     = count;
        if (expectCapacity > _capacity)
        {
            if (truncation)
            {
                pushCount = count - (expectCapacity - _capacity);
            }
            else
            {
                pushCount = 0;
            }
        }
        if (pushCount)
        {
            memcpy(_data.get() + _current, data, pushCount * sizeof(Value));
            _current += pushCount;
        }
        return pushCount;
    }

    //buffer要提前分配好足够的空间，truncation表示当不满足要求的数量时是否允许只返回部分，返回的是实际出队的元素个数
    size_t Pop(Value* buffer, size_t count, bool truncation = false)
    {
        std::lock_guard<Mutex> lock(_mutex);
        size_t                 popCount = count;
        if (count > _current)
        {
            if (truncation)
            {
                popCount = _current;
            }
            else
            {
                popCount = 0;
            }
        }
        if (popCount)
        {
            std::memcpy(buffer, _data.get(), sizeof(Value) * popCount);
            _current -= popCount;
            std::memmove(_data.get(), _data.get() + popCount, _current * sizeof(Value));
        }
        return popCount;
    }

    //直接返回智能指针，不允许返回部分元素
    std::shared_ptr<Value> PopShared(size_t count)
    {
        std::unique_lock<Mutex> lock(_mutex);
        if (count > _current)
        {
            return nullptr;
        }
        else
        {
            auto buffer = std::shared_ptr<Value>(new Value[count], std::default_delete<Value[]>());
            lock.unlock();
            if (count == Pop(buffer.get(), count, false))
            {
                return buffer;
            }
            else
            {
                return nullptr;
            }
        }
    }

    size_t Size() const
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _current;
    }

    size_t BufferSize() const
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _current * sizeof(Value);
    }

    void Clear()
    {
        std::lock_guard<Mutex> lock(_mutex);
        _current = 0;
    }

    size_t Capacity() const
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _capacity;
    }

    size_t BufferCapacity() const
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _capacity * sizeof(Value);
    }

    bool Empty()
    {
        std::lock_guard<Mutex> lock(_mutex);
        return 0 == _current;
    }
private:
    mutable Mutex            _mutex;
    std::unique_ptr<Value[]> _data;
    size_t                   _current;
    size_t                   _capacity;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
