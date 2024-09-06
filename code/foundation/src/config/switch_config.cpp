#include "zeus/foundation/config/switch_config.h"

#include <cassert>

#include <map>

#include <nlohmann/json.hpp>

#include "zeus/foundation/string/string_utils.h"

namespace zeus
{
struct SwitchConfigImpl
{
    std::map<std::string, ConfigPtr> configs;
    ConfigPtr                        defaultConfig;
    std::string                      label;
    std::string                      defaultLabel;
    bool                             partial = true;
};

SwitchConfig::SwitchConfig(const std::string& defaultLabel, bool partial) : _impl(std::make_unique<SwitchConfigImpl>())
{
    _impl->defaultLabel = defaultLabel;
    _impl->partial      = partial;
}
SwitchConfig::~SwitchConfig()
{
}
std::string SwitchConfig::Current() const
{
    return _impl->label;
}

void SwitchConfig::Switch(const std::string& label)
{
    _impl->label = label;
}
void SwitchConfig::AddConfig(const ConfigPtr& config, const std::string& label)
{
    _impl->configs.emplace(label, config);
    if (_impl->defaultLabel == label)
    {
        _impl->defaultConfig = config;
    }
}
void SwitchConfig::RemoveConfig(const std::string& label)
{
    if (_impl->defaultLabel == label)
    {
        _impl->defaultConfig.reset();
    }
    _impl->configs.erase(label);
}
ConfigPtr SwitchConfig::FindConfig(const std::string& label, bool partial) const
{
    if (!label.empty())
    {
        if (auto iter = _impl->configs.find(label); iter != _impl->configs.end())
        {
            return iter->second;
        }
        if (partial)
        {
            for (auto& item : _impl->configs)
            {
                if (std::string::npos != label.find(item.first))
                {
                    return item.second;
                }
            }
        }
    }
    return _impl->defaultConfig;
}
bool SwitchConfig::HasConfigValue(const std::string& key) const
{
    auto config = FindConfig(_impl->label, _impl->partial);
    if (config)
    {
        return config->HasConfigValue(key);
    }
    return false;
}
zeus::expected<ConfigValue, ConfigError> SwitchConfig::GetConfigValue(const std::string& key) const
{
    auto config = FindConfig(_impl->label, _impl->partial);
    if (config)
    {
        return config->GetConfigValue(key);
    }
    return zeus::unexpected(ConfigError::kNotFound);
}

std::vector<std::string> SwitchConfig::GetConfigKeys(const std::string& key) const
{
    auto config = FindConfig(_impl->label, _impl->partial);
    if (config)
    {
        return config->GetConfigKeys(key);
    }
    return std::vector<std::string>();
}
} // namespace zeus
