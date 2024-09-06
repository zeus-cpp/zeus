#pragma once

#include <map>
#include <vector>
#include <memory>
#include <type_traits>
#include "zeus/foundation/sync/mutex_object.hpp"

namespace zeus
{
template<typename KeyType, typename ValueType>
class ConcurrentMapBase
{
    using DataType = std::map<KeyType, ValueType>;
public:
    ValueType Get(const KeyType& key)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        auto iter = this->_data->find(key);
        if (iter != this->_data->end())
        {
            return iter->second;
        }
        else
        {
            return ValueType();
        }
    }
    bool Remove(const KeyType& key)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        auto size = this->_data->erase(key);
        return size > 0;
    }
    bool Has(const KeyType& key)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        return this->_data->count(key) > 0;
    }
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

    std::vector<typename DataType::mapped_type> Values()
    {
        std::vector<typename DataType::mapped_type> values;
        MUTEX_OBJECT_LOCK(this->_data);
        for (auto& iter : this->_data.Value())
        {
            values.emplace_back(iter.second);
        }
        return values;
    }

    std::vector<KeyType> Keys()
    {
        std::vector<KeyType> keys;
        MUTEX_OBJECT_LOCK(this->_data);
        for (auto& iter : this->_data.Value())
        {
            keys.emplace_back(iter.first);
        }
        return keys;
    }

    void Swap(DataType& other)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->swap(other);
    }

    DataType& Data() { return this->_data; }

    std::mutex& Mutex() { return this->_data.Mutex(); }



protected:
    zeus::MutexObject<DataType> _data;
};

template<typename KeyType, typename ValueType, bool shared = false>
class ConcurrentMap : public ConcurrentMapBase<KeyType, ValueType>
{
};

template<typename KeyType, typename ValueType>
class ConcurrentMap<KeyType, ValueType, false> : public ConcurrentMapBase<KeyType, ValueType>

{
public:
    bool Set(const KeyType& key, const ValueType& value, bool cover = true)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        if (cover)
        {
            auto iter = this->_data->find(key);
            if (iter != this->_data->end())
            {
                iter->second = value;
            }
            else
            {
                this->_data->emplace(key, value);
            }
            return true;
        }
        else
        {
            auto ret = this->_data->emplace(key, value);
            return ret.second;
        }
    }

    bool Set(const KeyType& key, ValueType&& value, bool cover = true)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        if (cover)
        {
            auto iter = this->_data->find(key);
            if (iter != this->_data->end())
            {
                iter->second = std::forward<ValueType>(value);
            }
            else
            {
                this->_data->emplace(key, std::forward<ValueType>(value));
            }
            return true;
        }
        else
        {
            auto ret = this->_data->emplace(key, std::forward<ValueType>(value));
            return ret.second;
        }
    }
};

template<typename KeyType, typename ValueType>
class ConcurrentMap<KeyType, ValueType, true> : public ConcurrentMapBase<KeyType, std::shared_ptr<ValueType>>
{
public:
    bool Set(const KeyType& key, const ValueType& value, bool cover = true)
    {
        auto data = std::make_shared<ValueType>(value);
        return Set(key, data, cover);
    }

    bool Set(const KeyType& key, ValueType&& value, bool cover = true)
    {
        auto data = std::make_shared<ValueType>(std::forward<ValueType>(value));
        return Set(key, data, cover);
    }

    bool Set(const KeyType& key, const std::shared_ptr<ValueType>& value, bool cover = true)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        if (cover)
        {
            auto iter = this->_data->find(key);
            if (iter != this->_data->end())
            {
                iter->second = value;
            }
            else
            {
                this->_data->emplace(key, value);
            }
            return true;
        }
        else
        {
            auto ret = this->_data->emplace(key, value);
            return ret.second;
        }
    }
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
