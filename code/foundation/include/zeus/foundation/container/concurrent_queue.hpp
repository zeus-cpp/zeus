#pragma once

#include <queue>
#include <memory>
#include <type_traits>
#include "zeus/foundation/sync/mutex_object.hpp"

namespace zeus
{
template<typename ValueType>
class ConcurrentQueueBase
{
    using DataType = std::queue<ValueType>;
public:

    void Clear()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->clear();
    }
    size_t Size()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        return this->_data->size();
    }

    bool Empty()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        return this->_data->empty();
    }

    DataType Values()
    {
        DataType values;
        MUTEX_OBJECT_LOCK(this->_data);
        values = this->_data.Value();
        return values;
    }

    void Swap(DataType& other)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->swap(other);
    }

    DataType& Data() { return this->_data; }

    std::mutex& Mutex() { return this->_data.Mutex(); }

    void Pop()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->pop();
    }

    ValueType Back()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        return this->_data->back();
    }

    ValueType Front()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        return this->_data->front();
    }

    ValueType PopAndFront()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        auto value = this->_data->front();
        this->_data->pop();
        return value;
    }

protected:
    zeus::MutexObject<DataType> _data;
};

template<typename ValueType, bool shared = false>
class ConcurrentQueue : public ConcurrentQueueBase<ValueType>
{
};

template<typename ValueType>
class ConcurrentQueue<ValueType, false> : public ConcurrentQueueBase<ValueType>
{
public:
    void Push(const ValueType& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(value);
    }

    void Push(ValueType&& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(std::forward<ValueType>(value));
    }

    template<typename... Args>
    void Emplace(Args&&... args)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(std::forward<Args>(args)...);
    }
};

template<typename ValueType>
class ConcurrentQueue<ValueType, true> : public ConcurrentQueueBase<std::shared_ptr<ValueType>>
{
public:
    void Push(const ValueType& value)
    {
        auto data = std::make_shared<ValueType>(value);
        Push(data);
    }

    void Push(ValueType&& value)
    {
        auto data = std::make_shared<ValueType>(std::forward<ValueType>(value));
        Push(data);
    }

    void Push(const std::shared_ptr<ValueType>& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(value);
    }

    template<typename... Args>
    void Emplace(Args&&... args)
    {
        auto data = std::make_shared<ValueType>(std::forward<Args>(args)...);
        Push(data);
    }
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
