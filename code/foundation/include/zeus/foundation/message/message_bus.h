#pragma once
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <any>
#include <vector>

namespace zeus
{

struct MessageBusImpl;
class MessageBus
{
public:
    MessageBus(bool concurrent = false);
    ~MessageBus();
    MessageBus(const MessageBus&)            = delete;
    MessageBus(MessageBus&&)                 = delete;
    MessageBus& operator=(const MessageBus&) = delete;
    template<typename... Args>
    size_t Subscribe(const std::string& topic, const std::function<void(Args...)>& callback)
    {
        return Subscribe(topic, std::any(callback));
    }
    bool UnSubscribe(size_t id);
    template<typename... Args>
    size_t Publish(const std::string& topic, Args... args)
    {
        size_t count   = 0;
        using CallType = std::function<void(Args...)>;
        auto callbacks = GetCallback(topic);
        for (const auto& callback : callbacks)
        {
            CallType* call = nullptr;
            try
            {
                call = std::any_cast<CallType>(callback.get());
            }
            catch (const std::bad_any_cast&)
            {
            }
            if (call)
            {
                ++count;
                try
                {
                    (*call)(args...);
                }
                catch (const std::exception& e)
                {
                    const auto& exceptionCallback = GetExceptionCallback();
                    if (exceptionCallback)
                    {
                        exceptionCallback(e);
                    }
                }
            }
        }
        return count;
    }
    void SetExceptionCallcack(const std::function<void(const std::exception& exception)>& callback);
    const std::function<void(const std::exception& exception)>& GetExceptionCallback() const;
private:
    std::vector<std::shared_ptr<std::any>> GetCallback(const std::string& topic);
    size_t                                 Subscribe(const std::string& topic, std::any&& callback);

private:
    std::unique_ptr<MessageBusImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
