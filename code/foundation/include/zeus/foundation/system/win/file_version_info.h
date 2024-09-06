#pragma once
#ifdef _WIN32
#include <string>
#include <memory>
#include <optional>
#include <string_view>
#include <filesystem>
#include <system_error>
#include <zeus/expected.hpp>
#include "zeus/foundation/string/version.h"

namespace zeus
{
struct WinFileInfoImpl;
class WinFileInfo
{
public:
    WinFileInfo(const WinFileInfo&) = delete;
    WinFileInfo(WinFileInfo&& other) noexcept;
    WinFileInfo& operator=(const WinFileInfo&) = delete;
    WinFileInfo& operator=(WinFileInfo&& other) noexcept;
    ~WinFileInfo();

    std::optional<std::string> GetExternFileVersion() const;
    std::optional<std::string> GetExternProductVersion() const;
    std::optional<std::string> GetFileDescription() const;
    std::optional<std::string> GetInternalName() const;
    std::optional<std::string> GetCompanyName() const;
    std::optional<std::string> GetLegalCopyright() const;
    std::optional<std::string> GetOriginalFilename() const;
    std::optional<std::string> GetProductName() const;
    std::optional<std::string> GetCustomString(std::string_view key) const;
    uint16_t                   Language() const;
    uint16_t                   CodePage() const;
protected:
    WinFileInfo();
    friend class WinFileVersionInfo;
private:
    std::unique_ptr<WinFileInfoImpl> _impl;
};

struct WinFileVersionInfoImpl;
class WinFileVersionInfo
{
public:
    WinFileVersionInfo(const WinFileVersionInfo&) = delete;
    WinFileVersionInfo(WinFileVersionInfo&& other) noexcept;
    WinFileVersionInfo& operator=(const WinFileVersionInfo&) = delete;
    WinFileVersionInfo& operator=(WinFileVersionInfo&& other) noexcept;
    ~WinFileVersionInfo();
    std::optional<zeus::Version>    GetFileVersion() const;
    std::optional<zeus::Version>    GetProductVersion() const;
    const std::vector<WinFileInfo>& GetInfoList();
public:
    static zeus::expected<WinFileVersionInfo, std::error_code> Load(const std::filesystem::path& path);
protected:
    WinFileVersionInfo();
private:
    std::unique_ptr<WinFileVersionInfoImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
