#pragma once
#include <memory>

#include "zeus/foundation/config/config.h"
#include "zeus/foundation/serialization/serializer.h"

namespace zeus
{
struct BaseConfigImpl;
class GeneralConfig : public Config
{
public:
    GeneralConfig();
    GeneralConfig(const std::shared_ptr<Serializer>& serializer, bool autoSerialization = true);
    ~GeneralConfig() override;
    GeneralConfig(const GeneralConfig&)            = delete;
    GeneralConfig& operator=(const GeneralConfig&) = delete;
    GeneralConfig(GeneralConfig&&)                 = delete;
    GeneralConfig& operator=(GeneralConfig&&)      = delete;

    zeus::expected<void, ConfigError> Load();
    zeus::expected<void, ConfigError> Save();

    [[deprecated("use key \"\" instead")]] void      SetConfigMap(const ConfigMap& configMap);
    [[deprecated("use key \"\" instead")]] void      SetConfigMap(ConfigMap&& configMap);
    [[deprecated("use key \"\" instead")]] ConfigMap GetConfigMap() const;

    bool                                     HasConfigValue(const std::string& key) const override;
    zeus::expected<ConfigValue, ConfigError> GetConfigValue(const std::string& key) const override;
    zeus::expected<void, ConfigError>        SetConfigValue(const std::string& key, const ConfigValue& value) override;
    zeus::expected<void, ConfigError>        RemoveConfigValue(const std::string& key) override;
    std::vector<std::string>                 GetConfigKeys(const std::string& key) const override;

    size_t AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const override;
    size_t AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue& value)>& notify) const override;
    bool   RemoveChangeNotify(size_t id) const override;

private:
    std::unique_ptr<BaseConfigImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
