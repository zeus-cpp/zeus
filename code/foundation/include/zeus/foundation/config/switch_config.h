#pragma once

#include <memory>

#include "zeus/foundation/config/config.h"

namespace zeus
{
struct SwitchConfigImpl;
class SwitchConfig : public Config
{
public:
    SwitchConfig(const std::string& defaultLabel, bool partial = true);
    ~SwitchConfig() override;
    SwitchConfig(const SwitchConfig&)            = delete;
    SwitchConfig& operator=(const SwitchConfig&) = delete;
    SwitchConfig(SwitchConfig&&)                 = delete;
    SwitchConfig& operator=(SwitchConfig&&)      = delete;
    std::string   Current() const;
    void          Switch(const std::string& label);
    void          AddConfig(const ConfigPtr& config, const std::string& label);
    void          RemoveConfig(const std::string& label);
    ConfigPtr     FindConfig(const std::string& label, bool partial = true) const;
protected:
    bool                                     HasConfigValue(const std::string& key) const override;
    zeus::expected<ConfigValue, ConfigError> GetConfigValue(const std::string& key) const override;
    std::vector<std::string>                 GetConfigKeys(const std::string& key) const override;
private:
    std::unique_ptr<SwitchConfigImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
