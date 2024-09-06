#ifdef _WIN32
#include "zeus/foundation/system/win/file_version_info.h"
#include <Windows.h>
#include <fmt/format.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"

#pragma comment(lib, "version.lib")

namespace zeus
{
namespace
{

std::optional<VS_FIXEDFILEINFO*> GetFixInfo(const void* versionData)
{
    VS_FIXEDFILEINFO* info      = nullptr;
    UINT              querySize = 0;
    if (VerQueryValueW(versionData, LR"(\)", reinterpret_cast<LPVOID*>(&info), &querySize) && info)
    {
        return info;
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<std::string> GetVersionCustomString(std::wstring_view key, const std::wstring& languageCode, const void* versionData)
{
    if (languageCode.empty())
    {
        return std::nullopt;
    }
    std::wstring block(LR"(\StringFileInfo\)");
    block.reserve(block.size() + languageCode.size() + key.size() + 1);
    block.append(languageCode);
    block.append(LR"(\)");
    block.append(key);
    wchar_t* data      = nullptr;
    UINT     querySize = 0;
    if (VerQueryValueW(versionData, block.c_str(), reinterpret_cast<LPVOID*>(&data), &querySize) && data && querySize)
    {
        return CharsetUtils::UnicodeToUTF8(std::wstring(data));
    }
    else
    {
        return std::nullopt;
    }
}
} // namespace

struct WinFileInfoImpl
{
    std::shared_ptr<uint8_t[]> versionData;
    std::wstring               languageCode;
    uint16_t                   language;
    uint16_t                   codePage;
};

struct WinFileVersionInfoImpl
{
    std::vector<WinFileInfo>   infos;
    std::shared_ptr<uint8_t[]> versionData;
};

WinFileInfo::WinFileInfo() : _impl(std::make_unique<WinFileInfoImpl>())
{
}
WinFileInfo::WinFileInfo(WinFileInfo&& other) noexcept : _impl(std::move(other._impl))
{
}
WinFileInfo& WinFileInfo::operator=(WinFileInfo&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
WinFileInfo::~WinFileInfo()
{
}

std::optional<std::string> WinFileInfo::GetExternFileVersion() const
{
    return GetVersionCustomString(L"FileVersion", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetExternProductVersion() const
{
    return GetVersionCustomString(L"ProductVersion", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetFileDescription() const
{
    return GetVersionCustomString(L"FileDescription", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetInternalName() const
{
    return GetVersionCustomString(L"InternalName", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetCompanyName() const
{
    return GetVersionCustomString(L"CompanyName", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetLegalCopyright() const
{
    return GetVersionCustomString(L"LegalCopyright", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetOriginalFilename() const
{
    return GetVersionCustomString(L"OriginalFilename", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetProductName() const
{
    return GetVersionCustomString(L"ProductName", _impl->languageCode, _impl->versionData.get());
}

std::optional<std::string> WinFileInfo::GetCustomString(std::string_view key) const
{
    return GetVersionCustomString(CharsetUtils::UTF8ToUnicode(key), _impl->languageCode, _impl->versionData.get());
}

uint16_t WinFileInfo::Language() const
{
    return _impl->language;
}
uint16_t WinFileInfo::CodePage() const
{
    return _impl->codePage;
}

WinFileVersionInfo::WinFileVersionInfo() : _impl(std::make_unique<WinFileVersionInfoImpl>())
{
}
WinFileVersionInfo::WinFileVersionInfo(WinFileVersionInfo&& other) noexcept : _impl(std::move(other._impl))
{
}
WinFileVersionInfo& WinFileVersionInfo::operator=(WinFileVersionInfo&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
WinFileVersionInfo::~WinFileVersionInfo()
{
}

std::optional<zeus::Version> WinFileVersionInfo::GetFileVersion() const
{
    auto info = GetFixInfo(_impl->versionData.get());
    if (info.has_value())
    {
        auto verMajor = HIWORD((*info)->dwFileVersionMS);
        auto verMinor = LOWORD((*info)->dwFileVersionMS);
        auto verPatch = HIWORD((*info)->dwFileVersionLS);
        auto verBuild = LOWORD((*info)->dwFileVersionLS);

        return zeus::Version(verMajor, verMinor, verPatch, verBuild);
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<zeus::Version> WinFileVersionInfo::GetProductVersion() const
{
    auto info = GetFixInfo(_impl->versionData.get());
    if (info.has_value())
    {
        auto verMajor = HIWORD((*info)->dwProductVersionMS);
        auto verMinor = LOWORD((*info)->dwProductVersionMS);
        auto verPatch = HIWORD((*info)->dwProductVersionLS);
        auto verBuild = LOWORD((*info)->dwProductVersionLS);

        return zeus::Version(verMajor, verMinor, verPatch, verBuild);
    }
    else
    {
        return std::nullopt;
    }
}

const std::vector<WinFileInfo>& WinFileVersionInfo::GetInfoList()
{
    return _impl->infos;
}

zeus::expected<WinFileVersionInfo, std::error_code> WinFileVersionInfo::Load(const std::filesystem::path& path)
{
    DWORD handle   = 0;
    auto  filepath = path.wstring();
    if (StartWith(filepath, LR"(\??\)"))
    {
        filepath = filepath.substr(4);
    }
    const DWORD size = ::GetFileVersionInfoSizeW(filepath.c_str(), &handle);
    if (0 == size)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    std::shared_ptr<uint8_t[]> versionData(new uint8_t[size]);
    if (!GetFileVersionInfoW(filepath.c_str(), handle, size, versionData.get()))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    struct LanguageCodePage
    {
        WORD language = 0;
        WORD codePage = 0;
    };
    LanguageCodePage* translateData = nullptr;

    std::wstring languageCode;
    UINT         translateSize = 0;
    if (!VerQueryValueW(versionData.get(), LR"(\VarFileInfo\Translation)", reinterpret_cast<LPVOID*>(&translateData), &translateSize))
    {
        return zeus::unexpected(SystemError {ERROR_RESOURCE_TYPE_NOT_FOUND});
    }
    const size_t translateCount = translateSize / sizeof(LanguageCodePage);

    WinFileVersionInfo fileVersion;
    fileVersion._impl->versionData = std::move(versionData);
    fileVersion._impl->infos.reserve(translateCount);
    for (size_t index = 0; index < translateCount; ++index)
    {
        const auto& translate = translateData[index];
        languageCode          = CharsetUtils::UTF8ToUnicode(fmt::format("{:04x}{:04x}", translate.language, translate.codePage));
        WinFileInfo info;
        info._impl->versionData  = fileVersion._impl->versionData;
        info._impl->languageCode = languageCode;
        info._impl->language     = translate.language;
        info._impl->codePage     = translate.codePage;
        fileVersion._impl->infos.emplace_back(std::move(info));
    }

    return fileVersion;
}
} // namespace zeus
#endif
