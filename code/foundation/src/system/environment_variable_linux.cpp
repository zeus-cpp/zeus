#include "zeus/foundation/system/environment_variable.h"
#ifdef __linux__
#include <mutex>
#include <shared_mutex>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/string_utils.h"

namespace
{
std::shared_mutex& GetEnvironmentMutex()
{
    static std::shared_mutex environmentMutex;
    return environmentMutex;
}
}

namespace zeus::EnvironmentVariable
{
std::optional<std::string> Get(std::string_view name)
{
    std::shared_lock lock(GetEnvironmentMutex());
    const char*      val = getenv(name.data());
    if (val)
    {
        return std::string(val);
    }
    else
    {
        return std::nullopt;
    }
}

bool Has(std::string_view name)
{
    std::shared_lock lock(GetEnvironmentMutex());
    return getenv(name.data()) != nullptr;
}

zeus::expected<void, std::error_code> Set(std::string_view name, std::string_view value)
{
    std::unique_lock lock(GetEnvironmentMutex());
    if (0 == setenv(name.data(), value.data(), 1))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

zeus::expected<void, std::error_code> Unset(std::string_view name)
{
    if (0 == unsetenv(name.data()))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

std::string ExpandEnvironmentVariableString(std::string_view data)
{
    const auto envParse = [](std::string_view key)
    {
        auto value = Get(key);
        if (value.has_value())
        {
            return std::move(value.value());
        }
        else
        {
            return std::string("${").append(key).append("}");
        }
    };
    return ExpandVariableString(data, envParse, "${", "}");
}

} // namespace zeus::EnvironmentVariable

#endif
