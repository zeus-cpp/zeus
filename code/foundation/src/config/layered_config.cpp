#include "zeus/foundation/config/layered_config.h"
#include <list>
#include <set>
#include <algorithm>
#include <functional>
#include <nlohmann/json.hpp>
#include "zeus/foundation/container/container_cast.hpp"
#include "zeus/foundation/container/callback_manager.hpp"
namespace
{
struct ConfigItem
{
    zeus::ConfigPtr config;
    int32_t         order;
    bool            writeable;
    std::string     label;
    size_t          notifyId = 0;
};

}
namespace zeus
{
struct LayeredConfigImpl
{
    std::list<ConfigItem>                                                    configs;
    NameCallbackManager<std::string, const ConfigPoint&, const ConfigValue&> changeNotifyManager =
        NameCallbackManager<std::string, const ConfigPoint&, const ConfigValue&>(0, true);
    const std::function<void(const ConfigPoint& key, const ConfigValue& value)> changeCallback =
        [this](const ConfigPoint& key, const ConfigValue& value)
    {
        changeNotifyManager.Call(key.to_string(), key, value);
        changeNotifyManager.Call("", key, value);
    };
};

LayeredConfig::LayeredConfig() : _impl(std::make_unique<LayeredConfigImpl>())
{
}
LayeredConfig::~LayeredConfig()
{
}

void LayeredConfig::AddConfig(const ConfigPtr& config, const std::string& label, bool writeable, int32_t order)
{
    auto iter = _impl->configs.begin();
    while (iter != _impl->configs.end() && iter->order < order)
    {
        ++iter;
    }
    auto reference = _impl->configs.emplace(
        iter,
        ConfigItem {
            config,
            order,
            writeable,
            label,
        }
    );
    reference->notifyId = config->AddChangeNotify(
        [this, config](const ConfigPoint& key, const ConfigValue& value)
        {
            for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
            {
                if (iter->config->HasConfigValue(key.to_string()))
                {
                    if (iter->config == config)
                    {
                        _impl->changeCallback(key, value);
                    }
                    break;
                }
            }
        }
    );
}

void LayeredConfig::RemoveConfig(const ConfigPtr& config)
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->config == config)
        {
            iter->config->RemoveChangeNotify(iter->notifyId);
            _impl->configs.erase(iter);
            break;
        }
    }
}

void LayeredConfig::RemoveConfig(const std::string& label)
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->label == label)
        {
            iter->config->RemoveChangeNotify(iter->notifyId);
            _impl->configs.erase(iter);
            break;
        }
    }
}

ConfigPtr LayeredConfig::FindConfig(const std::string& label) const
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->label == label)
        {
            return iter->config;
        }
    }
    return nullptr;
}

std::vector<ConfigPtr> LayeredConfig::GetAllConfig() const
{
    std::vector<ConfigPtr> configs;
    configs.reserve(_impl->configs.size());
    for (const auto& config : _impl->configs)
    {
        configs.emplace_back(config.config);
    }
    return configs;
}

bool LayeredConfig::HasConfigValue(const std::string& key) const
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->config->HasConfigValue(key))
        {
            return true;
        }
    }
    return false;
}
zeus::expected<ConfigValue, ConfigError> LayeredConfig::GetConfigValue(const std::string& key) const
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        auto value = iter->config->GetConfigValue(key);
        if (value.has_value())
        {
            return std::move(value);
        }
    }
    return zeus::unexpected(ConfigError::kNotFound);
}
zeus::expected<void, ConfigError> LayeredConfig::SetConfigValue(const std::string& key, const ConfigValue& value)
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->writeable && iter->config->HasConfigValue(key))
        {
            return iter->config->SetConfigValue(key, value);
        }
    }
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->writeable)
        {
            return iter->config->SetConfigValue(key, value);
        }
    }
    return zeus::unexpected(ConfigError::kLayeredNoWriteableConfigs);
}
zeus::expected<void, ConfigError> LayeredConfig::RemoveConfigValue(const std::string& key)
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (iter->writeable && iter->config->HasConfigValue(key))
        {
            return iter->config->RemoveConfigValue(key);
        }
    }
    return zeus::unexpected(ConfigError::kLayeredNoWriteableConfigs);
}
std::vector<std::string> LayeredConfig::GetConfigKeys(const std::string& key) const
{
    std::set<std::string> keys;
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        auto subKeys = iter->config->GetConfigKeys(key);
        for (const auto& subkey : subKeys)
        {
            keys.emplace(subkey);
        }
    }
    return SetToVector(keys);
}

size_t LayeredConfig::AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const
{
    return _impl->changeNotifyManager.AddCallback("", notify);
}

size_t LayeredConfig::AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue&)>& notify) const
{
    return _impl->changeNotifyManager.AddCallback(
        CasConfigPointer(key).to_string(), [notify](const ConfigPoint& /*key*/, const ConfigValue& value) { notify(value); }
    );
}

bool LayeredConfig::RemoveChangeNotify(size_t id) const
{
    return _impl->changeNotifyManager.RemoveCallback(id);
}

} // namespace zeus
