#ifdef _WIN32
#include "zeus/foundation/system/win/file_manifest.h"
#include <string>
#include <vector>
#include <list>
#include <cassert>
#include <Windows.h>
#include <tinyxml2.h>
#include "zeus/foundation/system/shared_library.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{

struct WinFileManifestImpl
{
    std::optional<WinFileManifest::Invoke> invoke;
    std::optional<bool>                    uiAccess;
};

BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCWSTR /*lpszType*/, LPWSTR lpszName, LONG_PTR lParam)
{
    WinFileManifestImpl* object = reinterpret_cast<WinFileManifestImpl*>(lParam);
    assert(object);
    HRSRC manifestSrc = ::FindResourceW(hModule, lpszName, RT_MANIFEST);
    do
    {
        if (!manifestSrc)
        {
            break;
        }
        HGLOBAL manifestGlobal = ::LoadResource(hModule, manifestSrc);
        if (!manifestGlobal)
        {
            break;
        }
        const auto* manifestData = ::LockResource(manifestGlobal);
        const auto  length       = SizeofResource(hModule, manifestSrc);
        if (!manifestData || !length)
        {
            break;
        }
        tinyxml2::XMLDocument doc;
        auto                  ret = doc.Parse(reinterpret_cast<const char*>(manifestData), length);
        if (tinyxml2::XMLError::XML_SUCCESS != ret)
        {
            break;
        }
        auto* ele = doc.FirstChildElement("assembly");
        if (!ele)
        {
            break;
        }
        ele = ele->FirstChildElement("trustInfo");
        if (!ele)
        {
            break;
        }
        ele = ele->FirstChildElement("security");
        if (!ele)
        {
            break;
        }
        ele = ele->FirstChildElement("requestedPrivileges");
        if (!ele)
        {
            break;
        }
        ele = ele->FirstChildElement("requestedExecutionLevel");
        if (!ele)
        {
            break;
        }
        const auto* invoke = ele->Attribute("level");
        if (IEqual(invoke, "asInvoker"))
        {
            object->invoke = WinFileManifest::Invoke::kAsInvoker;
        }
        else if (IEqual(invoke, "highestAvailable"))
        {
            object->invoke = WinFileManifest::Invoke::kHighestAvailable;
        }
        else if (IEqual(invoke, "requireAdministrator"))
        {
            object->invoke = WinFileManifest::Invoke::kRequireAdministrator;
        }

        const auto* uiAccess = ele->Attribute("uiAccess");
        if (IEqual(uiAccess, "true"))
        {
            object->uiAccess = true;
        }
        else if (IEqual(uiAccess, "false"))
        {
            object->uiAccess = true;
        }
    }
    while (false);
    return TRUE;
}

WinFileManifest::WinFileManifest() : _impl(std::make_unique<WinFileManifestImpl>())
{
}

WinFileManifest::WinFileManifest(WinFileManifest&& other) noexcept : _impl(std::move(other._impl))
{
}

WinFileManifest& WinFileManifest::operator=(WinFileManifest&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

WinFileManifest::~WinFileManifest()
{
}

std::optional<WinFileManifest::Invoke> WinFileManifest::GetInvoke() const
{
    return _impl->invoke;
}

std::optional<bool> WinFileManifest::IsUiAccess() const
{
    return _impl->uiAccess;
}

zeus::expected<WinFileManifest, std::error_code> WinFileManifest::Load(const std::filesystem::path& path)
{
    auto filepath = path.wstring();
    if (StartWith(filepath, LR"(\??\)"))
    {
        filepath = filepath.substr(4);
    }
    auto library = SharedLibrary::Load(filepath, LOAD_LIBRARY_AS_DATAFILE);
    if (library.has_value())
    {
        WinFileManifest manifest;
        ::EnumResourceNamesW(library.value(), RT_MANIFEST, EnumResNameProc, reinterpret_cast<LONG_PTR>(manifest._impl.get()));
        return manifest;
    }
    return zeus::unexpected(library.error());
}
} // namespace zeus

#endif //_WIN32
