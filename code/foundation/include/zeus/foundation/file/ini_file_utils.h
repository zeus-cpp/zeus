#pragma once
#include <filesystem>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <string_view>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{
zeus::expected<std::string, std::error_code> GetIniFileValue(
    const std::filesystem::path& path, const std::string_view& section, const std::string_view& key
);
zeus::expected<std::vector<std::string>, std::error_code> GetIniFileValues(
    const std::filesystem::path& path, const std::string_view& section, const std::string_view& key
);
zeus::expected<std::set<std::string>, std::error_code>                   GetIniFileSections(const std::filesystem::path& path);
zeus::expected<std::multimap<std::string, std::string>, std::error_code> GetIniFileData(
    const std::filesystem::path& path, const std::string_view& section
);
zeus::expected<std::map<std::string, std::string>, std::error_code> GetIniFileUniqueData(
    const std::filesystem::path& path, const std::string_view& section
);

}
#include "zeus/foundation/core/zeus_compatible.h"
