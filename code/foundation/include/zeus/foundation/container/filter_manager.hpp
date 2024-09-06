#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace zeus
{
template<typename... Arg>
class FilterManager
{
    using FilterFunction = typename std::function<bool(Arg...)>;
public:
    FilterManager(bool concurrent = false) : _id {1}, _concurrent(concurrent) {}
    ~FilterManager() {};
    FilterManager(const FilterManager&)            = delete;
    FilterManager& operator=(const FilterManager&) = delete;
    FilterManager(FilterManager&&)                 = delete;
    FilterManager& operator=(FilterManager&&)      = delete;
    size_t         AddFilter(const FilterFunction& callback)
    {
        std::unique_lock lock(_mutex, std::defer_lock);
        if (_concurrent)
        {
            lock.lock();
        }
        _filters[_id] = callback;
        return _id++;
    }
    bool RemoveFilter(size_t id)
    {
        std::unique_lock lock(_mutex, std::defer_lock);
        if (_concurrent)
        {
            lock.lock();
        }
        return _filters.erase(id) > 0;
    }
    bool CheckFilter(Arg... args)
    {
        std::unique_lock lock(_mutex, std::defer_lock);
        if (_concurrent)
        {
            lock.lock();
        }
        for (const auto& filter : _filters)
        {
            if (!filter.second(args...))
            {
                return false;
            }
        }
        return true;
    }

    void Clear()
    {
        std::unique_lock lock(_mutex, std::defer_lock);
        if (_concurrent)
        {
            lock.lock();
        }
        _filters.clear();
    }
private:
    size_t                                     _id;
    bool                                       _concurrent;
    std::mutex                                 _mutex;
    std::unordered_map<size_t, FilterFunction> _filters;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
