#include "zeus/foundation/config/config.h"

#include <nlohmann/json.hpp>

#include "zeus/foundation/string/string_utils.h"

using namespace nlohmann;

namespace zeus
{

Config::Config()
{
}
Config::~Config()
{
}

zeus::expected<void, ConfigError> Config::SetConfigValue(const std::string& /*key*/, const ConfigValue& /*value*/)
{
    return zeus::unexpected(ConfigError::kOperationUnsupported);
}

zeus::expected<void, ConfigError> Config::RemoveConfigValue(const std::string& /*key*/)
{
    return zeus::unexpected(ConfigError::kOperationUnsupported);
}

std::vector<std::string> Config::GetConfigKeys(const std::string& /*key*/) const
{
    return std::vector<std::string>();
}

size_t Config::AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& /*notify*/) const
{
    return 0;
}

size_t Config::AddChangeNotify(const std::string& /*key*/, const std::function<void(const ConfigValue& value)>& /*notify*/) const
{
    return 0;
}

size_t Config::AddChangeNotify(const std::string& key, const std::function<void()>& notify)
{
    return AddChangeNotify(key, [notify](const ConfigValue&) { notify(); });
}

bool Config::RemoveChangeNotify(size_t /*id*/) const
{
    return false;
}

ConfigPoint Config::CasConfigPointer(const std::string& key)
{
    if (key.empty() || StartWith(key, "/"))
    {
        return json_pointer<std::string>(key);
    }
    else
    {
        if (std::string::npos != key.find('.'))
        {
            return json_pointer<std::string>("/" + Replace(key, ".", "/"));
        }
        else
        {
            return json_pointer<std::string>("/" + key);
        }
    }
}

} // namespace zeus
