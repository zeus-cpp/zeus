#pragma once

#include <vector>
#include <memory>
#include <type_traits>
#include "zeus/foundation/sync/mutex_object.hpp"

namespace zeus
{
template<typename ValueType>
class ConcurrentVectorBase
{
public:
    using DataType = std::vector<ValueType>;
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

    void Reserve(size_t capacity)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->reserve(capacity);
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

    void PopBack()
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->pop_back();
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

    ValueType At(size_t index)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        return this->_data->at(index);
    }
protected:
    zeus::MutexObject<DataType> _data;
};

template<typename ValueType, bool shared = false>
class ConcurrentVector : public ConcurrentVectorBase<ValueType>
{
};

template<typename ValueType>
class ConcurrentVector<ValueType, false> : public ConcurrentVectorBase<ValueType>
{
public:
    void PushBack(const ValueType& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace_back(value);
    }

    void PushBack(ValueType&& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace_back(std::forward<ValueType>(value));
    }

    template<typename... Args>
    void EmplaceBack(Args&&... args)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace_back(std::forward<Args>(args)...);
    }
};

template<typename ValueType>
class ConcurrentVector<ValueType, true> : public ConcurrentVectorBase<std::shared_ptr<ValueType>>
{
public:
    void PushBack(const ValueType& value)
    {
        auto data = std::make_shared<ValueType>(value);
        PushBack(data);
    }

    void PushBack(ValueType&& value)
    {
        auto data = std::make_shared<ValueType>(std::forward<ValueType>(value));
        PushBack(data);
    }

    void PushBack(const std::shared_ptr<ValueType>& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace_back(value);
    }

    template<typename... Args>
    void EmplaceBack(Args&&... args)
    {
        auto data = std::make_shared<ValueType>(std::forward<Args>(args)...);
        PushBack(data);
    }
};

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
