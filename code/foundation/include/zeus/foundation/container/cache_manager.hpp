#pragma once
#include <functional>
#include <mutex>
#include <optional>
#include <memory>
#include <unordered_map>
#include <type_traits>

namespace zeus
{
template<typename Key, typename Value, typename Mutex = std::mutex>
class MultiCacheManager
{
public:
    using ValueType = Value;
    using KeyType   = Key;
    MultiCacheManager() {}
    ~MultiCacheManager() {}
    MultiCacheManager(const MultiCacheManager&)            = delete;
    MultiCacheManager(MultiCacheManager&&)                 = delete;
    MultiCacheManager& operator=(const MultiCacheManager&) = delete;
    MultiCacheManager& operator=(MultiCacheManager&&)      = delete;

    void SetCreateCallback(const std::function<std::optional<Value>(const Key& key)>& createCallback) { _createCallback = createCallback; }

    void SetChangeCallback(const std::function<void(const Key& key, const std::optional<const Value>& value)>& changeCallback)
    {
        _changeCallback = changeCallback;
    }

    size_t Size()
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _data.size();
    }

    bool Empty()
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _data.empty();
    }
    bool Has(const Key& key)
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _data.find(key) != _data.end();
    }

    bool Set(const Key& key, const Value& value, bool cover = true, bool notify = true) { return Set(key, Value(value), cover, notify); }

    //使用Set要求Value必须支持相等比较
    bool Set(const Key& key, Value&& value, bool cover = true, bool notify = true)
    {
        std::lock_guard<Mutex> lock(_mutex);
        bool                   change = true;
        auto                   iter   = _data.find(key);
        if (iter != _data.end())
        {
            if (iter->second == value)
            {
                return false;
            }

            if (cover)
            {
                iter->second = std::move(value);
            }
            else
            {
                //如果不覆盖，要重置改变标识
                change = false;
            }
        }
        else
        {
            iter = _data.emplace(key, std::move(value)).first;
        }
        if (notify && change && _changeCallback)
        {
            _changeCallback(key, iter->second);
        }
        return change;
    }

    std::optional<Value> Get(const Key& key)
    {
        std::lock_guard<Mutex> lock(_mutex);

        auto iter = _data.find(key);
        if (iter != _data.end())
        {
            return iter->second;
        }
        else
        {
            if (_createCallback)
            {
                auto value = _createCallback(key);
                if (value.has_value())
                {
                    auto result = _data.insert_or_assign(key, std::move(value));
                    if (_changeCallback)
                    {
                        _changeCallback(key, result.first->second);
                    }
                    return result.first->second;
                }
            }
            return std::nullopt;
        }
    }

    bool Remove(const Key& key, bool notify = true)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto                   iter = _data.find(key);
        if (iter != _data.end())
        {
            _data.erase(iter);
            if (notify && _changeCallback)
            {
                _changeCallback(key, std::nullopt);
            }
            return true;
        }
        return false;
    }

    //notify 是否要调用change回调，只有有缓存值的键时候才会回调
    void Clear(bool notify = false)
    {
        std::lock_guard<Mutex> lock(_mutex);
        if (notify && _changeCallback)
        {
            for (const auto& item : _data)
            {
                _changeCallback(item.first, std::nullopt);
            }
        }
        _data.clear();
    }

    void Notify(const std::function<void(const Key& key, const std::optional<const Value>& value)>& changeCallback)
    {
        std::lock_guard<Mutex> lock(_mutex);
        if (changeCallback)
        {
            for (const auto& item : _data)
            {
                changeCallback(item.first, item.second);
            }
        }
    }
private:
    Mutex                                                                        _mutex;
    std::unordered_map<Key, std::optional<Value>>                                _data;
    std::function<std::optional<Value>(const Key& key)>                          _createCallback;
    std::function<void(const Key& key, const std::optional<const Value>& value)> _changeCallback;
};

template<typename Value, typename Mutex = std::mutex>
class CacheManager
{
public:
    using ValueType = Value;
    CacheManager() {}
    ~CacheManager() {}
    CacheManager(const CacheManager&)            = delete;
    CacheManager(CacheManager&&)                 = delete;
    CacheManager& operator=(const CacheManager&) = delete;
    CacheManager& operator=(CacheManager&&)      = delete;

    void SetCreateCallback(const std::function<std::optional<Value>()>& createCallback) { _createCallback = createCallback; }

    void SetChangeCallback(const std::function<void(const std::optional<const Value>& value)>& changeCallback) { _changeCallback = changeCallback; }

    bool Empty()
    {
        std::lock_guard<Mutex> lock(_mutex);
        return !_data.has_value();
    }

    bool Set(Value&& value, bool cover = true, bool notify = true)
    {
        std::lock_guard<Mutex> lock(_mutex);
        bool                   change = true;
        if (_data.has_value())
        {
            if (_data.value() == value)
            {
                return false;
            }
            if (cover)
            {
                _data = std::move(value);
            }
            else
            {
                //如果不覆盖，要重置改变标识
                change = false;
            }
        }
        else
        {
            _data = std::move(value);
        }
        if (notify && change && _changeCallback)
        {
            _changeCallback(_data);
        }
        return change;
    }

    bool Set(const Value& value, bool cover = true, bool notify = true) { return Set(Value(value), cover, notify); }

    bool Reset(bool notify = true)
    {
        std::lock_guard<Mutex> lock(_mutex);
        if (_data.has_value())
        {
            _data.reset();
            if (notify && _changeCallback)
            {
                _changeCallback(std::nullopt);
            }
            return true;
        }
        return false;
    }

    std::optional<Value> Get()
    {
        std::lock_guard<Mutex> lock(_mutex);
        if (_data.has_value())
        {
            return _data;
        }
        else
        {
            if (_createCallback)
            {
                _data = _createCallback();
                if (_changeCallback)
                {
                    _changeCallback(_data);
                }
                return _data;
            }
            return std::nullopt;
        }
    }

    //notify 是否要调用change回调，只有有缓存值的时候才会回调
    void Clear(bool notify = false)
    {
        std::lock_guard<Mutex> lock(_mutex);
        if (_data.has_value() && notify && _changeCallback)
        {
            _changeCallback(std::nullopt);
        }
        _data.reset();
    }

private:
    Mutex                                                        _mutex;
    std::optional<Value>                                         _data;
    std::function<std::optional<Value>()>                        _createCallback;
    std::function<void(const std::optional<const Value>& value)> _changeCallback;
};

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
