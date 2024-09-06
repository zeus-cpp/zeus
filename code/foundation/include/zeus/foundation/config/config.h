#pragma once

#include <memory>
#include <string>
#include <functional>

#include <zeus/expected.hpp>

#include "zeus/foundation/config/config_commom.h"

namespace zeus
{
class Config
{
public:
    Config();
    virtual ~Config();
    Config(const Config&)            = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&)                 = delete;
    Config& operator=(Config&&)      = delete;

    virtual bool                                     HasConfigValue(const std::string& key) const = 0;
    virtual zeus::expected<ConfigValue, ConfigError> GetConfigValue(const std::string& key) const = 0;
    virtual zeus::expected<void, ConfigError>        SetConfigValue(const std::string& key, const ConfigValue& value);
    virtual zeus::expected<void, ConfigError>        RemoveConfigValue(const std::string& key);
    virtual std::vector<std::string>                 GetConfigKeys(const std::string& key = "") const;

    virtual size_t AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const;
    virtual size_t AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue& value)>& notify) const;
    virtual size_t AddChangeNotify(const std::string& key, const std::function<void()>& notify);
    virtual bool   RemoveChangeNotify(size_t id) const;

public:
    // FIXME typo here
    static ConfigPoint CasConfigPointer(const std::string& key);
};

using ConfigPtr = std::shared_ptr<Config>;
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
