#pragma once

#include <map>
#include <set>
#include <iterator>
#include <vector>
#include <memory>
#include <type_traits>
#include "zeus/foundation/sync/mutex_object.hpp"

namespace zeus
{
template<typename KeyType, typename ValueType>
class ConcurrentMultiMapBase
{
    using DataType = std::multimap<KeyType, ValueType>;
public:
    std::vector<typename DataType::mapped_type> Get(const KeyType& key)
    {
        std::vector<typename DataType::mapped_type> result;
        MUTEX_OBJECT_LOCK(this->_data);
        result.reserve(this->_data->count(key));
        auto range = this->_data->equal_range(key);
        for (auto& iter = range.first; iter != range.second; iter++)
        {
            result.emplace_back(iter->second);
        }
        return result;
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
        std::set<KeyType> keys;
        {
            MUTEX_OBJECT_LOCK(this->_data);
            for (auto& iter : this->_data.Value())
            {
                if (!keys.count(iter.first))
                {
                    keys.emplace(iter.first);
                }
            }
        }

        std::vector<KeyType> result;
        result.reserve(keys.size());
        std::copy(keys.begin(), keys.end(), std::back_inserter(result));
        return result;
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
class ConcurrentMultiMap : public ConcurrentMultiMapBase<KeyType, ValueType>
{
};

template<typename KeyType, typename ValueType>
class ConcurrentMultiMap<KeyType, ValueType, false> : public ConcurrentMultiMapBase<KeyType, ValueType>
{
public:
    void Set(const KeyType& key, const ValueType& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(key, value);
    }

    void Set(const KeyType& key, ValueType&& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(key, std::forward<ValueType>(value));
    }
};

template<typename KeyType, typename ValueType>
class ConcurrentMultiMap<KeyType, ValueType, true> : public ConcurrentMultiMapBase<KeyType, std::shared_ptr<ValueType>>
{
public:
    void Set(const KeyType& key, const ValueType& value)
    {
        auto data = std::make_shared<ValueType>(value);
        Set(key, data);
    }

    void Set(const KeyType& key, ValueType&& value)
    {
        auto data = std::make_shared<ValueType>(std::forward<ValueType>(value));
        Set(key, data);
    }

    void Set(const KeyType& key, const std::shared_ptr<ValueType>& value)
    {
        MUTEX_OBJECT_LOCK(this->_data);
        this->_data->emplace(key, value);
    }
};

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
