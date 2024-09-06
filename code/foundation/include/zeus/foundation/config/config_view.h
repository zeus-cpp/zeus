#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "zeus/foundation/config/config.h"

namespace zeus
{
struct ConfigViewImpl;
class ConfigView
{
public:
    ConfigView(Config& config, const std::string& prefix = "");
    ~ConfigView();
    ConfigView(const ConfigView&)            = delete;
    ConfigView& operator=(const ConfigView&) = delete;
    ConfigView(ConfigView&&)                 = delete;
    ConfigView& operator=(ConfigView&&)      = delete;

    Config&                           GetConfig() const;
    std::string                       TranslateKey(const std::string& key) const;
    bool                              Has(const std::string& key) const;
    zeus::expected<void, ConfigError> Remove(const std::string& key);
    std::vector<std::string>          Keys(const std::string& key) const;

    zeus::expected<std::string, ConfigError> ExpandValue(const std::string& value) const;

    template<typename Type>
    zeus::expected<Type, ConfigError> Get(const std::string& key) const;

    template<typename Type, typename Arg>
    Type Get(const std::string& key, Arg&& defaultValue) const;

    template<typename Type>
    zeus::expected<void, ConfigError> GetTo(const std::string& key, Type& value) const;

    template<typename Type>
    zeus::expected<void, ConfigError> Set(const std::string& key, const Type& value);

private:
    std::unique_ptr<ConfigViewImpl> _impl;
};

template<typename Type>
zeus::expected<Type, ConfigError> ConfigView::Get(const std::string& key) const
{
    return GetConfig()
        .GetConfigValue(TranslateKey(key))
        .and_then(
            [](ConfigValue const& value) -> zeus::expected<Type, ConfigError>
            {
                try
                {
                    return value.get<Type>();
                }
                catch (...)
                {
                    return zeus::unexpected(ConfigError::kSerializationError);
                }
            }
        );
}

template<typename Type, typename Arg>
Type ConfigView::Get(const std::string& key, Arg&& defaultValue) const
{
    return Get<Type>(key).value_or(std::forward<Arg>(defaultValue));
}

template<typename Type>
zeus::expected<void, ConfigError> ConfigView::GetTo(const std::string& key, Type& value) const
{
    return GetConfig()
        .GetConfigValue(TranslateKey(key))
        .and_then(
            [&value](ConfigValue const& place) -> zeus::expected<void, ConfigError>
            {
                try
                {
                    place.get_to<Type>(value);
                    return {};
                }
                catch (...)
                {
                    return zeus::unexpected(ConfigError::kSerializationError);
                }
            }
        );
}

template<typename Type>
zeus::expected<void, ConfigError> ConfigView::Set(const std::string& key, const Type& value)
{
    return GetConfig().SetConfigValue(TranslateKey(key), value);
}

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
