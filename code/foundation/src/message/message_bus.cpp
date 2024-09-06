#include "zeus/foundation/message/message_bus.h"
#include <shared_mutex>
#include <mutex>
#include <list>
#include <cassert>
#include <unordered_set>
#include <unordered_map>

namespace zeus
{
struct CallbackNode
{
    std::shared_mutex mutex;
    std::any          callback;
    std::string       topic;
    size_t            id = 0;
};
struct MessageBusImpl
{
    std::mutex                                                mutex;
    std::unordered_map<size_t, std::shared_ptr<CallbackNode>> callbacks;
    std::unordered_multimap<std::string, size_t>              topicIndex;
    size_t                                                    idGenerater = 0;
    std::function<void(const std::exception& exception)>      exceptionCallback;
    bool                                                      concurrent = false;
};
MessageBus::MessageBus(bool concurrent) : _impl(std::make_unique<MessageBusImpl>())
{
    _impl->concurrent = concurrent;
}
MessageBus::~MessageBus()
{
}

bool MessageBus::UnSubscribe(size_t id)
{
    std::shared_ptr<CallbackNode> node;
    {
        std::unique_lock lock(_impl->mutex);
        {
            auto iter = _impl->callbacks.find(id);
            if (iter != _impl->callbacks.end())
            {
                node = iter->second; //将node先拷贝出来，以防被销毁
                _impl->callbacks.erase(iter);
            }
        }
        if (node)
        {
            auto range = _impl->topicIndex.equal_range(node->topic);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                if (iter->second == id)
                {
                    _impl->topicIndex.erase(iter);
                    break;
                }
            }
        }
    }
    if (node)
    {
        //确保在node销毁时不处于执行状态
        std::unique_lock lock(node->mutex);
        node->callback = nullptr;

        return true;
    }
    else
    {
        return false;
    }
}
void MessageBus::SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback)
{
    _impl->exceptionCallback = callback;
}

const std::function<void(const std::exception& exception)>& MessageBus::GetExceptionCallback() const
{
    return _impl->exceptionCallback;
}

std::vector<std::shared_ptr<std::any>> MessageBus::GetCallback(const std::string& topic)
{
    std::list<std::shared_ptr<CallbackNode>> temp;
    {
        std::unique_lock lock(_impl->mutex);
        auto             range = _impl->topicIndex.equal_range(topic);
        for (auto iter = range.first; iter != range.second; ++iter)
        {
            auto node = _impl->callbacks.find(iter->second);
            if (node != _impl->callbacks.end())
            {
                temp.emplace_back(node->second);
            }
        }
    }
    std::vector<std::shared_ptr<std::any>> callbacks;
    for (auto& node : temp)
    {
        std::shared_lock shareLock(node->mutex, std::defer_lock);
        std::unique_lock uniqueLock(node->mutex, std::defer_lock);
        if (_impl->concurrent)
        {
            shareLock.lock();
        }
        else
        {
            uniqueLock.lock();
        }
        callbacks.emplace_back(
            &node->callback, [shareLock = std::move(shareLock), uniqueLock = std::move(uniqueLock)](std::any*)
            { assert(shareLock.owns_lock() || uniqueLock.owns_lock()); }
        );
    }
    return callbacks;
}

size_t MessageBus::Subscribe(const std::string& topic, std::any&& callback)
{
    auto node      = std::make_shared<CallbackNode>();
    node->callback = std::move(callback);
    node->topic    = topic;
    std::unique_lock lock(_impl->mutex);
    {
        node->id = _impl->idGenerater++;
        _impl->callbacks.emplace(node->id, node);
        _impl->topicIndex.emplace(topic, node->id);
    }
    return node->id;
}
} // namespace zeus
