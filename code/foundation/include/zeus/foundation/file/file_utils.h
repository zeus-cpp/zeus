#pragma once

#include <string>
#include <fstream>
#include <set>
#include <filesystem>
#include <functional>
#include <string_view>
#include <system_error>
#include <memory>
#include <cstdint>
#include <zeus/expected.hpp>

namespace zeus
{
zeus::expected<void, std::error_code> FileEachLine(
    const std::filesystem::path& path, const std::function<bool(std::string_view line)>& callback, char delim
);
zeus::expected<void, std::error_code> FileEachLine(const std::filesystem::path& path, const std::function<bool(std::string_view line)>& callback);

zeus::expected<void, std::error_code> FileEachLineKVData(
    const std::filesystem::path& path, std::string_view delim, const std::function<bool(std::string_view key, std::string_view value)>& callback,
    const std::set<std::string>& ignorePrefixs = {"#"}
);

zeus::expected<void, std::error_code> FileEachLineKVData(
    const std::filesystem::path& path, std::string_view delim, const std::function<bool(std::string_view key, std::string_view value)>& callback,
    const std::function<bool(std::string_view line)>& invalidCallback, const std::set<std::string>& ignorePrefixs = {"#"}
);

zeus::expected<std::string, std::error_code> FileContent(const std::filesystem::path& path, bool binary = false);

zeus::expected<uint64_t, std::error_code> GetDirectoryRegularFileSize(
    const std::filesystem::path& directory, bool recursive = false, bool followSymbolLinkDirectory = false, bool includeSymbolLinkFile = false
);

zeus::expected<std::set<std::filesystem::path>, std::error_code> GetDirectoryRegularFiles(
    const std::filesystem::path& directory, const std::function<bool(const std::filesystem::path& path)>& filter, bool recursive = false,
    bool followSymbolLinkDirectory = false, bool includeSymbolLinkFile = false
);

zeus::expected<std::set<std::filesystem::path>, std::error_code> GetDirectoryRegularFiles(
    const std::filesystem::path& directory, bool recursive = false, bool followSymbolLinkDirectory = false, bool includeSymbolLinkFile = false
);

zeus::expected<std::set<std::filesystem::path>, std::error_code> GetDirectoryRegularFiles(
    const std::filesystem::path& directory, const std::string& suffix, bool recursive = false, bool followSymbolLinkDirectory = false,
    bool includeSymbolLinkFile = false
);

zeus::expected<void, std::error_code> CreateWriteableDirectory(const std::filesystem::path& path);

zeus::expected<bool, std::error_code> FileEqual(std::ifstream& file1, std::ifstream& file2);
zeus::expected<bool, std::error_code> FileEqual(const std::filesystem::path& path1, const std::filesystem::path& path2);

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
