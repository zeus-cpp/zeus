#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

namespace zeus
{
enum class ConfigError
{
    kOperationUnsupported,
    kNotFound,
    kSerializationError,
    kUnseirializable,
    kExpandingDepthLimited,

    kLayeredNoWriteableConfigs,
};

using ConfigMap   = nlohmann::json;
using ConfigValue = nlohmann::json;
using ConfigPoint = nlohmann::json_pointer<std::string>;

inline constexpr auto ConfigErrorToString(ConfigError const error) -> auto
{
    switch (error)
    {
    case ConfigError::kOperationUnsupported:
        return "operation unsupported";
    case ConfigError::kNotFound:
        return "not found";
    case ConfigError::kSerializationError:
        return "serialization error";
    case ConfigError::kUnseirializable:
        return "unseirializable";
    case ConfigError::kExpandingDepthLimited:
        return "expanding depth limited";
    case ConfigError::kLayeredNoWriteableConfigs:
        return "the layered cnofig has no writeable configs";
    default:
        return "unknown config error";
    }
}

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
