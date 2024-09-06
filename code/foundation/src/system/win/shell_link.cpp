#include "zeus/foundation/system/win/shell_link.h"
#ifdef _WIN32
#include <fstream>
#include <array>
#include <cstring>
#include <cassert>
#include <ShlObj.h>
#include <atlbase.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/core/win/com_error.h"
#include "zeus/foundation/resource/win/com_init.h"
#include "zeus/foundation/thread/thread_checker.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/byte/byte_order.h"

namespace zeus
{

struct WinShellLinkImpl
{
    ComInit              comInit;
    CComPtr<IShellLinkW> shellLink;
    ThreadChecker        checker;
};

WinShellLink::WinShellLink() : _impl(std::make_unique<WinShellLinkImpl>())
{
    assert(_impl->checker.IsCurrent());
}
WinShellLink::WinShellLink(WinShellLink&& other) noexcept : _impl(std::move(other._impl))
{
    assert(_impl->checker.IsCurrent());
}
WinShellLink& WinShellLink::operator=(WinShellLink&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
WinShellLink::~WinShellLink()
{
}
zeus::expected<void, std::error_code> WinShellLink::SetPath(const std::string& path)
{
    assert(_impl->checker.IsCurrent());

    HRESULT hr = _impl->shellLink->SetPath(CharsetUtils::UTF8ToUnicode(path).c_str());
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<std::string, std::error_code> WinShellLink::GetPath()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellPath[MAX_PATH] = {0};
    HRESULT hr                  = _impl->shellLink->GetPath(shellPath, MAX_PATH, nullptr, 0);
    if (SUCCEEDED(hr))
    {
        return CharsetUtils::UnicodeToUTF8(shellPath);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinShellLink::GetRawPath()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellPath[MAX_PATH] = {0};
    HRESULT hr                  = _impl->shellLink->GetPath(shellPath, MAX_PATH, nullptr, SLGP_RAWPATH);
    if (SUCCEEDED(hr))
    {
        return CharsetUtils::UnicodeToUTF8(shellPath);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<void, std::error_code> WinShellLink::SetRelativePath(const std::string& path)
{
    assert(_impl->checker.IsCurrent());

    HRESULT hr = _impl->shellLink->SetRelativePath(CharsetUtils::UTF8ToUnicode(path).c_str(), 0);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<std::string, std::error_code> WinShellLink::GetRelativePath()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellPath[MAX_PATH] = {0};
    HRESULT hr                  = _impl->shellLink->GetPath(shellPath, MAX_PATH, nullptr, SLGP_RELATIVEPRIORITY);
    if (SUCCEEDED(hr))
    {
        return CharsetUtils::UnicodeToUTF8(shellPath);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<void, std::error_code> WinShellLink::SetArguments(const std::string& arguments)
{
    assert(_impl->checker.IsCurrent());
    HRESULT hr = _impl->shellLink->SetArguments(CharsetUtils::UTF8ToUnicode(arguments).c_str());
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<std::string, std::error_code> WinShellLink::GetArguments()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellArguments[INFOTIPSIZE] = {0};
    HRESULT hr                          = _impl->shellLink->GetArguments(shellArguments, INFOTIPSIZE);
    if (SUCCEEDED(hr))
    {
        return CharsetUtils::UnicodeToUTF8(shellArguments);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<void, std::error_code> WinShellLink::SetDescription(const std::string& description)
{
    assert(_impl->checker.IsCurrent());
    HRESULT hr = _impl->shellLink->SetDescription(CharsetUtils::UTF8ToUnicode(description).c_str());
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<std::string, std::error_code> WinShellLink::GetDescription()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellDescription[INFOTIPSIZE] = {0};
    HRESULT hr                            = _impl->shellLink->GetDescription(shellDescription, INFOTIPSIZE);
    if (SUCCEEDED(hr))
    {
        return CharsetUtils::UnicodeToUTF8(shellDescription);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<void, std::error_code> WinShellLink::SetWorkingDirectory(const std::string& directory)
{
    assert(_impl->checker.IsCurrent());
    HRESULT hr = _impl->shellLink->SetWorkingDirectory(CharsetUtils::UTF8ToUnicode(directory).c_str());
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<std::string, std::error_code> WinShellLink::GetWorkingDirectory()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellWorkingDirectory[INFOTIPSIZE] = {0};
    HRESULT hr                                 = _impl->shellLink->GetWorkingDirectory(shellWorkingDirectory, INFOTIPSIZE);
    if (SUCCEEDED(hr))
    {
        return CharsetUtils::UnicodeToUTF8(shellWorkingDirectory);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<void, std::error_code> WinShellLink::SetIconLocation(const std::string& location, size_t index)
{
    assert(_impl->checker.IsCurrent());
    HRESULT hr = _impl->shellLink->SetIconLocation(CharsetUtils::UTF8ToUnicode(location).c_str(), index);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<std::pair<std::string, size_t>, std::error_code> WinShellLink::GetIconLocation()
{
    assert(_impl->checker.IsCurrent());
    wchar_t shellIconLocation[INFOTIPSIZE] = {0};
    int     shellIconIndex                 = 0;
    HRESULT hr                             = _impl->shellLink->GetIconLocation(shellIconLocation, INFOTIPSIZE, &shellIconIndex);
    if (SUCCEEDED(hr))
    {
        return std::make_pair(CharsetUtils::UnicodeToUTF8(shellIconLocation), shellIconIndex);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinShellLink::SetShowCommand(int command)
{
    assert(_impl->checker.IsCurrent());
    HRESULT hr = _impl->shellLink->SetShowCmd(command);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}

zeus::expected<int, std::error_code> WinShellLink::GetShowCommand()
{
    assert(_impl->checker.IsCurrent());
    int     showCommand = 0;
    HRESULT hr          = _impl->shellLink->GetShowCmd(&showCommand);
    if (SUCCEEDED(hr))
    {
        return showCommand;
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinShellLink::Load(const std::filesystem::path& path)
{
    CComPtr<IPersistFile> persist;
    HRESULT               hr = _impl->shellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&persist));
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }

    hr = persist->Load(path.c_str(), STGM_READ); //加载文件
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}
zeus::expected<void, std::error_code> WinShellLink::Save(const std::filesystem::path& path)
{
    CComPtr<IPersistFile> persist;
    HRESULT               hr = _impl->shellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&persist));
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }

    hr = persist->Save(path.c_str(), TRUE);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    return {};
}

zeus::expected<WinShellLink, std::error_code> WinShellLink::Create()
{
    WinShellLink link;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<LPVOID*>(&link._impl->shellLink));
    if (SUCCEEDED(hr))
    {
        return std::move(link);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<std::string, std::error_code> WinShellLink::GetLocalBasePath(const std::filesystem::path& path)
{
#pragma pack(push)
#pragma pack(1)
    struct ShellLinkHeader
    {
        uint32_t size;
        uint32_t clsid[4];
        uint32_t flags;
        uint32_t fileAttributes;
        uint64_t creationTime;
        uint64_t accessTime;
        uint64_t writeTime;
        uint32_t fileSize;
        uint32_t iconIndex;
        uint32_t showCommand;
        uint16_t hotKey;
        uint16_t reserved0;
        uint32_t reserved1;
        uint32_t reserved2;
    };
    struct LinkInfo
    {
        uint32_t headerSize;
        uint32_t flags;
        uint32_t volumeIDOffset;
        uint32_t localBasePathOffset;
        uint32_t commonNetworkRelativeLinkOffset;
        uint32_t commonPathSuffixOffset;
    };
#pragma pack(pop)
    static constexpr std::array<uint8_t, 16> kLinkMagic = {0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                           0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46};
    std::ifstream                            file(path, std::ios::binary);
    if (!file)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    ShellLinkHeader header = {};
    if (file.read(reinterpret_cast<char*>(&header), sizeof(ShellLinkHeader)).eof())
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (header.size < sizeof(ShellLinkHeader))
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (0 != std::memcmp(kLinkMagic.data(), header.clsid, kLinkMagic.size()))
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (header.flags & 0xF1000000) //前5位必须为0
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (0 == (header.flags & 0x2)) //第2位必须为1
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (header.flags & 0x1) //第1位表示是否有IDList
    {
        uint16_t linkTargetIDListSize = 0;
        file.read(reinterpret_cast<char*>(&linkTargetIDListSize), sizeof(uint16_t));
        file.seekg(linkTargetIDListSize, std::ios::cur);
    }
    if (file.eof())
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    uint32_t linkInfoSize = 0;
    if (file.read(reinterpret_cast<char*>(&linkInfoSize), sizeof(uint32_t)).eof())
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (linkInfoSize < sizeof(LinkInfo))
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    const auto linkInfoBuffer = std::make_unique<uint8_t[]>(linkInfoSize + sizeof(uint32_t));
    if (file.read(reinterpret_cast<char*>(linkInfoBuffer.get() + sizeof(uint32_t)), linkInfoSize).eof())
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }

    auto* linkInfo = reinterpret_cast<LinkInfo*>(linkInfoBuffer.get() + sizeof(uint32_t));

    if (0 == (linkInfo->flags & 0x1))
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (0 == linkInfo->localBasePathOffset)
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    if (linkInfo->localBasePathOffset >= linkInfoSize)
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    const auto len =
        strnlen_s(reinterpret_cast<char*>(linkInfoBuffer.get() + linkInfo->localBasePathOffset), linkInfoSize - linkInfo->localBasePathOffset);
    if (len == linkInfoSize - linkInfo->localBasePathOffset)
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_INVALID_BLOCK}));
    }
    return CharsetUtils::ANSIToUTF8(std::string_view(reinterpret_cast<char*>(linkInfoBuffer.get() + linkInfo->localBasePathOffset), len));
}
} // namespace zeus
#endif