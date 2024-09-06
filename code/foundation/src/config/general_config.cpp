#include "zeus/foundation/config/general_config.h"
#include <shared_mutex>
#include <list>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "zeus/foundation/container/callback_manager.hpp"
using namespace nlohmann;
namespace zeus
{
struct BaseConfigImpl
{
    std::shared_mutex                                                        mutex;
    json                                                                     data;
    NameCallbackManager<std::string, const ConfigPoint&, const ConfigValue&> changeNotifyManager =
        NameCallbackManager<std::string, const ConfigPoint&, const ConfigValue&>(0, true);
    std::shared_ptr<Serializer> serializer;
    bool                        autoSerialization = false;
};

GeneralConfig::GeneralConfig() : _impl(std::make_unique<BaseConfigImpl>())
{
    _impl->changeNotifyManager.SetExceptionCallcack([](const std::exception&) {});
}

GeneralConfig::GeneralConfig(const std::shared_ptr<Serializer>& serializer, bool autoSerialization) : GeneralConfig()
{
    _impl->serializer        = serializer;
    _impl->autoSerialization = autoSerialization;
    if (autoSerialization)
    {
        Load();
    }
}
GeneralConfig::~GeneralConfig()
{
}

zeus::expected<void, ConfigError> GeneralConfig::Load()
{
    if (_impl->serializer)
    {
        auto deserializationResult = _impl->serializer->Load();
        if (!deserializationResult)
        {
            return zeus::unexpected(ConfigError::kSerializationError);
        }
        ConfigMap data = nlohmann::json::parse(deserializationResult.value(), nullptr, false, true);
        if (data.is_discarded())
        {
            return zeus::unexpected(ConfigError::kSerializationError);
        }
        {
            std::unique_lock lock(_impl->mutex);
            _impl->data.swap(data);
            return {};
        }
    }
    return zeus::unexpected(ConfigError::kUnseirializable);
}

zeus::expected<void, ConfigError> GeneralConfig::Save()
{
    if (_impl->serializer)
    {
        std::string buffer;
        {
            std::shared_lock lock(_impl->mutex);
            buffer = _impl->data.dump();
        }
        if (!_impl->serializer->Save(buffer.data(), buffer.size()))
        {
            return zeus::unexpected(ConfigError::kSerializationError);
        }
        return {};
    }
    return zeus::unexpected(ConfigError::kUnseirializable);
}

void GeneralConfig::SetConfigMap(const ConfigMap& configMap)
{
    std::unique_lock lock(_impl->mutex);
    _impl->data = configMap;
}

void GeneralConfig::SetConfigMap(ConfigMap&& configMap)
{
    std::unique_lock lock(_impl->mutex);
    _impl->data = std::move(configMap);
}

ConfigMap GeneralConfig::GetConfigMap() const
{
    std::shared_lock lock(_impl->mutex);
    return _impl->data;
}

bool GeneralConfig::HasConfigValue(const std::string& key) const
{
    auto const       point = CasConfigPointer(key);
    std::shared_lock lock(_impl->mutex);
    return _impl->data.contains(point) && !_impl->data.at(point).is_null();
}

zeus::expected<ConfigValue, ConfigError> GeneralConfig::GetConfigValue(const std::string& key) const
{
    auto const       point = CasConfigPointer(key);
    std::shared_lock lock(_impl->mutex);
    if (_impl->data.contains(point))
    {
        ConfigValue value = _impl->data.at(point);
        if (value.is_null())
        {
            return zeus::unexpected(ConfigError::kNotFound);
        }
        return std::move(value);
    }
    return zeus::unexpected(ConfigError::kNotFound);
}

zeus::expected<void, ConfigError> GeneralConfig::SetConfigValue(const std::string& key, const ConfigValue& value)
{
    zeus::expected<void, ConfigError> result;

    auto const       point = CasConfigPointer(key);
    std::unique_lock lock(_impl->mutex);
    if (_impl->data.contains(point))
    {
        bool  change    = false;
        auto& reference = _impl->data.at(point);
        change          = reference != value;
        if (change)
        {
            reference = value;
            lock.unlock();
            if (_impl->autoSerialization)
            {
                result = Save();
            }
            _impl->changeNotifyManager.Call(point.to_string(), point, value);
            _impl->changeNotifyManager.Call("", point, value);
        }
    }
    else
    {
        lock.unlock();
        assert(!point.empty()); // root 应该永远走不到这个分支
        auto subkeys = [&point]() -> std::list<std::string>
        {
            std::list<std::string> subkeys;
            auto                   pointCopy = point;
            while (!pointCopy.empty())
            {
                subkeys.emplace_front(pointCopy.back());
                pointCopy.pop_back();
            }
            return subkeys;
        }();
        assert(!subkeys.empty());
        auto lastKey = subkeys.back();
        subkeys.pop_back();
        lock.lock();
        json* currentData = &_impl->data;
        for (const auto& subkey : subkeys)
        {
            auto iter = currentData->find(subkey);
            if (iter != currentData->end())
            {
                currentData = &iter.value();
            }
            else
            {
                currentData = &currentData->emplace(subkey, json::object()).first.value();
            }
        }
        if (lastKey.empty())
        {
            *currentData = value;
        }
        else
        {
            currentData->emplace(lastKey, value).first.value();
        }
        lock.unlock();
        if (_impl->autoSerialization)
        {
            result = Save();
        }
        _impl->changeNotifyManager.Call(point.to_string(), point, value);
        _impl->changeNotifyManager.Call("", point, value);
    }

    return result;
}

zeus::expected<void, ConfigError> GeneralConfig::RemoveConfigValue(const std::string& key)
{
    auto const point = CasConfigPointer(key);

    std::unique_lock lock(_impl->mutex);
    if (point.empty())
    {
        _impl->data = nullptr;
    }
    else
    {
        if (_impl->data.contains(point))
        {
            auto& parent = _impl->data.at(point.parent_pointer());
            parent.erase(point.back());
        }
        else
        {
            return zeus::unexpected(ConfigError::kNotFound);
        }
    }
    lock.unlock();

    zeus::expected<void, ConfigError> saveResult;
    if (_impl->autoSerialization)
    {
        saveResult = Save();
    }
    _impl->changeNotifyManager.Call(point.to_string(), point, json());
    _impl->changeNotifyManager.Call("", point, json());

    return saveResult;
}
std::vector<std::string> GeneralConfig::GetConfigKeys(const std::string& key) const
{
    std::vector<std::string> keys;
    auto const               point = CasConfigPointer(key);
    std::shared_lock         lock(_impl->mutex);
    if (_impl->data.contains(point))
    {
        auto& place = _impl->data.at(point);
        if (place.is_object())
        {
            keys.reserve(place.size());
            for (const auto& item : place.items())
            {
                if (!item.value().is_null())
                {
                    keys.emplace_back(item.key());
                }
            }
        }
    }
    return keys;
}
size_t GeneralConfig::AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const
{
    return _impl->changeNotifyManager.AddCallback("", notify);
}
size_t GeneralConfig::AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue& value)>& notify) const
{
    return _impl->changeNotifyManager.AddCallback(
        CasConfigPointer(key).to_string(), [notify](const ConfigPoint& /*key*/, const ConfigValue& value) { notify(value); }
    );
}
bool GeneralConfig::RemoveChangeNotify(size_t id) const
{
    return _impl->changeNotifyManager.RemoveCallback(id);
}
} // namespace zeus
