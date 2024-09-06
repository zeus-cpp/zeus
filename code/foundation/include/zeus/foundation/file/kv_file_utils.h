#pragma once

#include <vector>
#include <string_view>
#include <filesystem>
#include <map>
#include <set>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{

zeus::expected<std::string, std::error_code> GetKVFileValue(const std::filesystem::path &path, std::string_view key, std::string_view delim = "=");
zeus::expected<std::vector<std::string>, std::error_code> GetKVFileValues(
    const std::filesystem::path &path, std::string_view key, std::string_view delim = "="
);

zeus::expected<std::string, std::error_code> GetKVFileValueUnquoted(
    const std::filesystem::path &path, std::string_view key, std::string_view delim = "="
);

zeus::expected<std::multimap<std::string, std::string>, std::error_code> GetKVFileData(
    const std::filesystem::path &path, std::string_view delim = "="
);
zeus::expected<std::map<std::string, std::string>, std::error_code> GetKVFileUniqueData(
    const std::filesystem::path &path, std::string_view delim = "="
);

zeus::expected<std::multimap<std::string, std::string>, std::error_code> PickKVFileData(
    const std::filesystem::path &path, const std::set<std::string> &keys, std::string_view delim = "="
);
zeus::expected<std::map<std::string, std::string>, std::error_code> PickKVFileUniqueData(
    const std::filesystem::path &path, const std::set<std::string> &keys, std::string_view delim = "="
);

}

#include "zeus/foundation/core/zeus_compatible.h"
