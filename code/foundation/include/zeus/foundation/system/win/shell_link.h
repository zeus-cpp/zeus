#pragma once
#ifdef _WIN32
#include <memory>
#include <optional>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{
struct WinShellLinkImpl;
class WinShellLink
{
public:

    WinShellLink(const WinShellLink&)            = delete;
    WinShellLink& operator=(const WinShellLink&) = delete;
    WinShellLink(WinShellLink&& other) noexcept;
    WinShellLink& operator=(WinShellLink&& other) noexcept;
    ~WinShellLink();

    zeus::expected<void, std::error_code>                           SetPath(const std::string& path);
    zeus::expected<std::string, std::error_code>                    GetPath();
    zeus::expected<std::string, std::error_code>                    GetRawPath();
    zeus::expected<void, std::error_code>                           SetRelativePath(const std::string& path);
    zeus::expected<std::string, std::error_code>                    GetRelativePath();
    zeus::expected<void, std::error_code>                           SetArguments(const std::string& arguments);
    zeus::expected<std::string, std::error_code>                    GetArguments();
    zeus::expected<void, std::error_code>                           SetDescription(const std::string& description);
    zeus::expected<std::string, std::error_code>                    GetDescription();
    zeus::expected<void, std::error_code>                           SetWorkingDirectory(const std::string& directory);
    zeus::expected<std::string, std::error_code>                    GetWorkingDirectory();
    zeus::expected<void, std::error_code>                           SetIconLocation(const std::string& location, size_t index = 0);
    zeus::expected<std::pair<std::string, size_t>, std::error_code> GetIconLocation();
    zeus::expected<void, std::error_code>                           SetShowCommand(int command);
    zeus::expected<int, std::error_code>                            GetShowCommand();

    zeus::expected<void, std::error_code> Load(const std::filesystem::path& path);
    zeus::expected<void, std::error_code> Save(const std::filesystem::path& path);
public:
    static zeus::expected<WinShellLink, std::error_code> Create();
    static zeus::expected<std::string, std::error_code>  GetLocalBasePath(const std::filesystem::path& path);
protected:
    WinShellLink();
private:
    std::unique_ptr<WinShellLinkImpl> _impl;
};

} // namespace zeus
#endif
#include "zeus/foundation/core/zeus_compatible.h"
