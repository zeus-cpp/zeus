#include "zeus/foundation/config/config_view.h"

namespace zeus
{
struct ConfigViewImpl
{
    Config&     config;
    std::string prefix;
};

ConfigView::ConfigView(Config& config, const std::string& prefix) : _impl(new ConfigViewImpl {config, prefix})
{
}

ConfigView::~ConfigView()
{
}

Config& ConfigView::GetConfig() const
{
    return _impl->config;
}

std::string ConfigView::TranslateKey(const std::string& key) const
{
    if (_impl->prefix.empty())
    {
        return key;
    }
    else if (key.empty() || key.front() == '/')
    {
        return _impl->prefix + key;
    }
    else
    {
        std::string completeKey;
        completeKey.reserve(_impl->prefix.size() + key.size() + 1);
        if (std::string::npos != key.find('.'))
        {
            completeKey.append(_impl->prefix).append(".").append(key);
        }
        else
        {
            completeKey.append(_impl->prefix).append("/").append(key);
        }
        return completeKey;
    }
}

bool ConfigView::Has(const std::string& key) const
{
    return GetConfig().HasConfigValue(TranslateKey(key));
}

zeus::expected<void, ConfigError> ConfigView::Remove(const std::string& key)
{
    return GetConfig().RemoveConfigValue(TranslateKey(key));
}

std::vector<std::string> ConfigView::Keys(const std::string& key) const
{
    return GetConfig().GetConfigKeys(TranslateKey(key));
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
zeus::expected<std::string, ConfigError> ConfigView::ExpandValue(const std::string& value) const
{
    std::function<zeus::expected<std::string, ConfigError>(const std::string& value, size_t depth)> internalExpand;
    internalExpand = [this, &internalExpand](const std::string& value, size_t depth) -> zeus::expected<std::string, ConfigError>
    {
        if (depth >= 10)
        {
            return zeus::unexpected(ConfigError::kExpandingDepthLimited);
        }
        std::string result;
        auto        iter = value.begin();
        auto        end  = value.end();
        while (iter != end)
        {
            if (*iter == '$')
            {
                ++iter;
                if (iter != end && *iter == '{')
                {
                    ++iter;
                    std::string prop;
                    while (iter != end && *iter != '}')
                    {
                        prop += *iter++;
                    }
                    if (iter != end)
                    {
                        ++iter;
                    }
                    std::string rawValue;
                    if (GetTo(prop, rawValue))
                    {
                        result.append(internalExpand(rawValue, depth + 1).value_or(""));
                    }
                    else
                    {
                        result.append("${");
                        result.append(prop);
                        result.append("}");
                    }
                }
                else
                {
                    result += '$';
                }
            }
            else
            {
                result += *iter++;
            }
        }
        return result;
    };
    return internalExpand(value, 0);
}
// NOLINTEND(readability-function-cognitive-complexity)

} // namespace zeus
