#pragma once
#include <zeus/foundation/config/config.h>

namespace zeus
{
struct BackupConfigImpl;
//备份配置，通过此配置对象进行的写入会对所有加入管理的配置写入。
class BackupConfig : public Config
{
public:
    BackupConfig();
    ~BackupConfig() override;
    BackupConfig(const BackupConfig&)            = delete;
    BackupConfig& operator=(const BackupConfig&) = delete;
    BackupConfig(BackupConfig&&)                 = delete;
    BackupConfig& operator=(BackupConfig&&)      = delete;

    //order越小优先级越高
    void AddConfig(const ConfigPtr& config, const std::string& label, int32_t order);
    void RemoveConfig(const ConfigPtr& config);
    void RemoveConfig(const std::string& label);

    bool                                     HasConfigValue(const std::string& key) const override;
    zeus::expected<ConfigValue, ConfigError> GetConfigValue(const std::string& key) const override;
    zeus::expected<void, ConfigError>        SetConfigValue(const std::string& key, const ConfigValue& value) override;
    zeus::expected<void, ConfigError>        RemoveConfigValue(const std::string& key) override;
    std::vector<std::string>                 GetConfigKeys(const std::string& key) const override;
    //通知会作用在主配置上
    size_t AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const override;
    size_t AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue&)>& notify) const override;
    bool   RemoveChangeNotify(size_t id) const override;
private:
    std::unique_ptr<BackupConfigImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
