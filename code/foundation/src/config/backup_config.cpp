#include "zeus/foundation/config/backup_config.h"
#include <list>
#include <set>
#include <algorithm>
#include <functional>
#include <nlohmann/json.hpp>
#include "zeus/foundation/container/container_cast.hpp"
#include "zeus/foundation/container/callback_manager.hpp"

namespace zeus
{
struct ConfigItem
{
    ConfigPtr   config;
    std::string label;
    int         order;
    size_t      notifyId = 0;
};

struct BackupConfigImpl
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

BackupConfig::BackupConfig() : _impl(std::make_unique<BackupConfigImpl>())
{
}
BackupConfig::~BackupConfig()
{
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        iter->config->RemoveChangeNotify(iter->notifyId);
    }
}
void BackupConfig::AddConfig(const ConfigPtr& config, const std::string& label, int32_t order)
{
    auto iter = _impl->configs.begin();
    while (iter != _impl->configs.end() && iter->order < order)
    {
        ++iter;
    }
    auto reference      = _impl->configs.emplace(iter, ConfigItem {config, label, order});
    reference->notifyId = config->AddChangeNotify(
        [this, config](const ConfigPoint& key, const ConfigValue& value)
        {
            if (config == _impl->configs.front().config)
            {
                _impl->changeCallback(key, value);
            }
        }
    );
}
void BackupConfig::RemoveConfig(const ConfigPtr& config)
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
void BackupConfig::RemoveConfig(const std::string& label)
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

bool BackupConfig::HasConfigValue(const std::string& key) const
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
zeus::expected<ConfigValue, ConfigError> BackupConfig::GetConfigValue(const std::string& key) const
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
zeus::expected<void, ConfigError> BackupConfig::SetConfigValue(const std::string& key, const ConfigValue& value)
{
    zeus::expected<void, ConfigError> result;
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (zeus::expected<void, ConfigError> curResult = iter->config->SetConfigValue(key, value); !curResult.has_value())
        {
            // 仅使用第一次失败的错误码
            if (result.has_value())
            {
                result = curResult;
            }
        }
    }
    return result;
}
zeus::expected<void, ConfigError> BackupConfig::RemoveConfigValue(const std::string& key)
{
    zeus::expected<void, ConfigError> result;
    for (auto iter = _impl->configs.begin(); iter != _impl->configs.end(); ++iter)
    {
        if (zeus::expected<void, ConfigError> curResult = iter->config->RemoveConfigValue(key); !curResult.has_value())
        {
            // 仅使用第一次失败的错误码
            if (result.has_value())
            {
                result = curResult;
            }
        }
    }
    return result;
}
std::vector<std::string> BackupConfig::GetConfigKeys(const std::string& key) const
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
size_t BackupConfig::AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const
{
    return _impl->changeNotifyManager.AddCallback("", notify);
}
size_t BackupConfig::AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue&)>& notify) const
{
    return _impl->changeNotifyManager.AddCallback(
        CasConfigPointer(key).to_string(), [notify](const ConfigPoint& /*key*/, const ConfigValue& value) { notify(value); }
    );
}
bool BackupConfig::RemoveChangeNotify(size_t id) const
{
    return _impl->changeNotifyManager.RemoveCallback(id);
}
} // namespace zeus
