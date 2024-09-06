#pragma once
#include <functional>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <list>
#include <memory>
#include <unordered_map>
#include <type_traits>

namespace zeus
{

template<typename... Arg>
class CallbackManager
{
    using CallbackFunction = typename std::function<void(Arg...)>;
public:
    CallbackManager(size_t maxCallback = 0, bool concurrent = false) : _maxCallback(maxCallback), _concurrent(concurrent) {}
    ~CallbackManager() { Clear(); }
    CallbackManager(const CallbackManager&)            = delete;
    CallbackManager(CallbackManager&&)                 = delete;
    CallbackManager& operator=(const CallbackManager&) = delete;

    size_t AddCallback(const CallbackFunction& callback) noexcept
    {
        auto node      = std::make_shared<CallbackNode>();
        node->callback = callback;
        {
            std::unique_lock lock(_mutex);
            if (_maxCallback && _callbacks.size() >= _maxCallback)
            {
                return 0;
            }
            node->id = ++_idGenerater;
            _callbacks.emplace(node->id, node);
        }
        return node->id;
    }

    size_t AddCallback(CallbackFunction&& callback) noexcept
    {
        auto node      = std::make_shared<CallbackNode>();
        node->callback = std::forward<CallbackFunction>(callback);
        {
            std::unique_lock lock(_mutex);
            if (_maxCallback && _callbacks.size() >= _maxCallback)
            {
                return 0;
            }
            node->id = ++_idGenerater;
            _callbacks.emplace(node->id, node);
        }
        return node->id;
    }

    //wait 表示是否等待回调结束
    bool RemoveCallback(size_t id, bool wait = true) noexcept
    {
        std::shared_ptr<CallbackNode> node;
        {
            std::unique_lock lock(_mutex);
            auto             iter = _callbacks.find(id);
            if (iter != _callbacks.end())
            {
                node = iter->second; //将node先拷贝出来，以防被销毁
                _callbacks.erase(iter);
            }
        }

        if (node)
        {
            if (wait)
            {
                //确保在node销毁时不处于执行状态
                std::unique_lock lock(node->mutex);
                node->callback = nullptr;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Call(Arg... args)
    {
        decltype(this->_callbacks) temp;
        {
            std::unique_lock lock(this->_mutex);
            temp = this->_callbacks;
        }
        if (!temp.empty())
        {
            for (const auto& iter : temp)
            {
                auto&            node = iter.second;
                std::shared_lock shareLock(node->mutex, std::defer_lock);
                std::unique_lock uniqueLock(node->mutex, std::defer_lock);
                if (this->_concurrent)
                {
                    shareLock.lock();
                }
                else
                {
                    uniqueLock.lock();
                }
                if (node->callback)
                {
                    try
                    {
                        node->callback(args...);
                    }
                    catch (const std::exception& e)
                    {
                        if (this->_exceptionCallback)
                        {
                            this->_exceptionCallback(e);
                        }
                        else
                        {
                            throw;
                        }
                    }
                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Empty() noexcept
    {
        std::unique_lock lock(_mutex);
        return _callbacks.empty();
    }

    size_t Size() noexcept
    {
        std::unique_lock lock(_mutex);
        return _callbacks.size();
    }

    std::set<size_t> CallbackIds() noexcept
    {
        std::unique_lock lock(_mutex);
        std::set<size_t> callbackIds;
        for (const auto& iter : _callbacks)
        {
            callbackIds.emplace(iter.first);
        }
        return callbackIds;
    }

    size_t Clear(bool wait = true) noexcept
    {
        auto callbackIds = CallbackIds();
        for (auto& id : callbackIds)
        {
            RemoveCallback(id, wait);
        }
        return callbackIds.size();
    }
    void SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback) { _exceptionCallback = callback; }
public:
    struct CallbackNode
    {
        std::shared_mutex mutex;
        CallbackFunction  callback;
        size_t            id = 0;
    };
protected:

    std::mutex                                                _mutex;
    std::unordered_map<size_t, std::shared_ptr<CallbackNode>> _callbacks;
    size_t                                                    _idGenerater = 0;
    size_t                                                    _maxCallback = 0;
    std::function<void(const std::exception& exception)>      _exceptionCallback;
    bool                                                      _concurrent = false;
};

template<typename NameType, typename... Arg>
class NameCallbackManager
{
    using CallbackFunction = typename std::function<void(Arg...)>;
public:
    NameCallbackManager(size_t maxCallback = 0, bool concurrent = false) : _maxCallback(maxCallback), _concurrent(concurrent) {}
    ~NameCallbackManager() { Clear(); }
    NameCallbackManager(const NameCallbackManager&)            = delete;
    NameCallbackManager(NameCallbackManager&&)                 = delete;
    NameCallbackManager& operator=(const NameCallbackManager&) = delete;

    size_t AddCallback(const NameType& name, const CallbackFunction& callback) noexcept
    {
        auto node      = std::make_shared<CallbackNode>();
        node->callback = callback;
        node->name     = name;
        {
            std::unique_lock lock(_mutex);
            if (_maxCallback && _nameIndex.count(name) >= _maxCallback)
            {
                return 0;
            }
            node->id = ++_idGenerater;
            _callbacks.emplace(node->id, node);
            _nameIndex.emplace(name, node->id);
        }
        return node->id;
    }

    size_t AddCallback(const NameType& name, CallbackFunction&& callback) noexcept
    {
        auto node      = std::make_shared<CallbackNode>();
        node->callback = std::forward<CallbackFunction>(callback);
        node->name     = name;
        {
            std::unique_lock lock(_mutex);
            if (_maxCallback && _nameIndex.count(name) >= _maxCallback)
            {
                return 0;
            }
            node->id = ++_idGenerater;
            _callbacks.emplace(node->id, node);
            _nameIndex.emplace(name, node->id);
        }
        return node->id;
    }

    bool RemoveCallback(size_t id, bool wait = true) noexcept
    {
        std::shared_ptr<CallbackNode> node;
        {
            std::unique_lock lock(_mutex);
            {
                auto iter = _callbacks.find(id);
                if (iter != _callbacks.end())
                {
                    node = iter->second; //将node先拷贝出来，以防被销毁
                    _callbacks.erase(iter);
                }
            }
            if (node)
            {
                auto range = _nameIndex.equal_range(node->name);
                for (auto iter = range.first; iter != range.second; ++iter)
                {
                    if (iter->second == id)
                    {
                        _nameIndex.erase(iter);
                        break;
                    }
                }
            }
        }
        if (node)
        {
            if (wait)
            {
                //确保在node销毁时不处于执行状态
                std::unique_lock lock(node->mutex);
                node->callback = nullptr;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool RemoveCallback(const NameType& name, bool wait = true) noexcept
    {
        std::list<size_t> ids;
        {
            std::unique_lock lock(_mutex);
            auto             range = _nameIndex.equal_range(name);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                ids.emplace_back(iter->second);
            }
        }
        if (ids.empty())
        {
            return false;
        }
        for (auto& id : ids)
        {
            RemoveCallback(id, wait);
        }
        return true;
    }

    bool Call(const NameType& name, Arg... args)
    {
        std::list<typename decltype(this->_callbacks)::mapped_type> temp;
        {
            std::unique_lock lock(this->_mutex);
            auto             range = this->_nameIndex.equal_range(name);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                auto node = this->_callbacks.find(iter->second);
                if (node != this->_callbacks.end())
                {
                    temp.emplace_back(node->second);
                }
            }
        }
        if (temp.empty())
        {
            return false;
        }
        for (auto& node : temp)
        {
            std::shared_lock shareLock(node->mutex, std::defer_lock);
            std::unique_lock uniqueLock(node->mutex, std::defer_lock);
            if (this->_concurrent)
            {
                shareLock.lock();
            }
            else
            {
                uniqueLock.lock();
            }
            if (node->callback)
            {
                try
                {
                    node->callback(args...);
                }
                catch (const std::exception& e)
                {
                    if (this->_exceptionCallback)
                    {
                        this->_exceptionCallback(e);
                    }
                    else
                    {
                        throw;
                    }
                }
            }
        }
        return true;
    }

    bool Empty() noexcept
    {
        std::unique_lock lock(_mutex);
        return _nameIndex.empty();
    }

    bool Empty(const NameType& name) noexcept
    {
        std::unique_lock lock(_mutex);
        return !_nameIndex.count(name);
    }

    size_t Size() noexcept
    {
        std::unique_lock lock(_mutex);
        return _callbacks.size();
    }

    size_t Size(const NameType& name) noexcept
    {
        std::unique_lock lock(_mutex);
        return _nameIndex.count(name);
    }

    std::set<size_t> CallbackIds(const NameType& name) noexcept
    {
        std::unique_lock lock(_mutex);
        std::set<size_t> callbackIds;
        auto             range = _nameIndex.equal_range(name);
        for (auto iter = range.first; iter != range.second; ++iter)
        {
            callbackIds.emplace(iter->second);
        }
        return callbackIds;
    }

    std::set<size_t> CallbackIds() noexcept
    {
        std::unique_lock lock(_mutex);
        std::set<size_t> callbackIds;
        for (auto& iter : _callbacks)
        {
            callbackIds.emplace(iter.first);
        }
        return callbackIds;
    }

    size_t Clear(bool wait = true) noexcept
    {
        auto callbackIds = CallbackIds();
        for (auto& id : callbackIds)
        {
            RemoveCallback(id, wait);
        }
        return callbackIds.size();
    }

    void SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback) { _exceptionCallback = callback; }

protected:
    struct CallbackNode
    {
        std::shared_mutex mutex;
        CallbackFunction  callback;
        NameType          name;
        size_t            id = 0;
    };
    std::mutex                                                _mutex;
    std::unordered_map<size_t, std::shared_ptr<CallbackNode>> _callbacks;
    std::unordered_multimap<NameType, size_t>                 _nameIndex;
    size_t                                                    _idGenerater = 0;
    size_t                                                    _maxCallback = 0;
    std::function<void(const std::exception& exception)>      _exceptionCallback;
    bool                                                      _concurrent = false;
};

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
