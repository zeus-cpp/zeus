#include "zeus/foundation/system/os.h"
#ifdef _WIN32
#include <sstream>
#include <locale>
#include <Windows.h>
#include "zeus/foundation/system/win/registry.h"
#include "zeus/foundation/system/shared_library.h"
#include "zeus/foundation/system/process.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/string/charset_utils.h"

#pragma comment(lib, "version.lib")

namespace
{

#define DETECTED_NOT_WIN_SERVER \
    if (IsWinServer())          \
    {                           \
        return false;           \
    }

#define DETECTEDT_WIN_SERVER \
    if (!IsWinServer())      \
    {                        \
        return false;        \
    }

const std::string kCurrentVersionRegister = R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)";

const std::string kSQMRegister = R"(SOFTWARE\Microsoft\SQMClient)";

OSVERSIONINFOEXW RtlGetVersion()
{
    typedef LONG(__stdcall * RtlGetVersionFunction)(OSVERSIONINFOEXW*);
    static auto rtlGetVersionFunction =
        zeus::SharedLibrary::Load(std::string("ntdll.dll"))
            .transform([](const zeus::SharedLibrary& library) { return library.GetTypeSymbol<RtlGetVersionFunction>("RtlGetVersion"); })
            .value_or(nullptr);

    OSVERSIONINFOEXW version    = OSVERSIONINFOEXW();
    version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    if (rtlGetVersionFunction)
    {
        rtlGetVersionFunction(&version);
    }
    return version;
}

const OSVERSIONINFOEXW& GetSystemVersion()
{
    static OSVERSIONINFOEXW cache(RtlGetVersion());
    return cache;
}

inline size_t GetSystemUBR()
{
    auto version = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kCurrentVersionRegister, true, false, zeus::WinRegistry::ViewType::WIN_KEY_64);
    return version.and_then([](const zeus::WinRegistry& version) { return version.GetDwordValue("UBR"); }).value_or(0);
}
} // namespace

namespace zeus::OS
{

std::string OsKernelName()
{
    const OSVERSIONINFOEXW& vi = GetSystemVersion();
    switch (vi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
        return "Windows 3.x";
    case VER_PLATFORM_WIN32_WINDOWS:
        return vi.dwMinorVersion == 0 ? "Windows 95" : "Windows 98";
    case VER_PLATFORM_WIN32_NT:
        return "Windows NT";
    default:
        return "Unknown";
    }
}
// NOLINTBEGIN(readability-function-cognitive-complexity)
std::string OsDisplayName()
{
    const OSVERSIONINFOEXW& vi = GetSystemVersion();
    std::string             name;
    switch (vi.dwMajorVersion)
    {
    case 10:
        switch (vi.dwMinorVersion)
        {
        case 0:
            if (vi.wProductType != VER_NT_WORKSTATION)
            {
                if (vi.dwBuildNumber < 17763)
                {
                    name = "Windows Server 2016";
                }
                else if (vi.dwBuildNumber < 20348)
                {
                    name = "Windows Server 2019";
                }
                else
                {
                    name = "Windows Server 2022";
                }
            }
            else
            {
                if (vi.dwBuildNumber < 22000)
                {
                    name = "Windows 10";
                }
                else
                {
                    name = "Windows 11";
                }
            }
            break;
        default:
            name = "Unknown";
        }
        break;
    case 6:
        switch (vi.dwMinorVersion)
        {
        case 0:
            name = vi.wProductType == VER_NT_WORKSTATION ? "Windows Vista" : "Windows Server 2008";
            break;
        case 1:
            name = vi.wProductType == VER_NT_WORKSTATION ? "Windows 7" : "Windows Server 2008 R2";
            break;
        case 2:
            name = vi.wProductType == VER_NT_WORKSTATION ? "Windows 8" : "Windows Server 2012";
            break;
        case 3:
            name = vi.wProductType == VER_NT_WORKSTATION ? "Windows 8.1" : "Windows Server 2012 R2";
            break;
        default:
            name = "Unknown";
        }
        break;
    case 5:
        switch (vi.dwMinorVersion)
        {
        case 0:
            name = "Windows 2000";
            break;
        case 1:
            name = "Windows XP";
            break;
        case 2:
            name = "Windows Server 2003/Windows Server 2003 R2";
            break;
        default:
            name = "Unknown";
        }
        break;
    default:
        name = "Unknown";
    }
    if (*vi.szCSDVersion)
    {
        auto pack = CharsetUtils::UnicodeToUTF8(vi.szCSDVersion);
        if (!pack.empty())
        {
            name.append(" " + pack);
        }
    }

    return name;
}

// NOLINTEND(readability-function-cognitive-complexity)

std::string OsProductName()
{
    return OsDisplayName() + " " + OsProductType();
}

zeus::Version OsVersion()
{
    static zeus::Version osVersion(
        GetSystemVersion().dwMajorVersion, GetSystemVersion().dwMinorVersion, GetSystemVersion().dwBuildNumber, GetSystemUBR()
    );
    return osVersion;
}

std::string OsVersionString()
{
    return OsVersion().ToString();
}

zeus::Version OsKernelVersion()
{
    return OsVersion();
}

std::string OsKernelVersionString()
{
    return OsVersionString();
}

std::string OsArchitecture()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        return "X86";
    case PROCESSOR_ARCHITECTURE_MIPS:
        return "MIPS";
    case PROCESSOR_ARCHITECTURE_ALPHA:
        return "ALPHA";
    case PROCESSOR_ARCHITECTURE_PPC:
        return "PPC";
    case PROCESSOR_ARCHITECTURE_ARM:
        return "ARM";
    case PROCESSOR_ARCHITECTURE_ARM64:
        return "ARM64";
    case PROCESSOR_ARCHITECTURE_IA64:
        return "IA64";
#ifdef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
    case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
        return "IA64/32";
#endif
#ifdef PROCESSOR_ARCHITECTURE_AMD64
    case PROCESSOR_ARCHITECTURE_AMD64:
        return "X64";
#endif
    default:
        return "Unknown";
    }
}

bool IsOs64Bits()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    return (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);
}

std::string OsProductType()
{
    auto version = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kCurrentVersionRegister, true, false, zeus::WinRegistry::ViewType::WIN_KEY_64);
    return version.and_then([](const zeus::WinRegistry& version) { return version.GetStringValue("EditionID"); }).value_or("");
}

std::string OsFullName()
{
    std::stringstream stream;
    stream.imbue(std::locale::classic());

    stream << OsProductName() << " ";
    if (IsWin10OrGreater())
    {
        stream << OsDisplayVersion() << "-";
    }
    stream << OsVersionString() << "-" << OsArchitecture();
    return stream.str();
}

std::chrono::system_clock::time_point OsBootTime()
{
    return std::chrono::system_clock::now() - std::chrono::duration_cast<std::chrono::system_clock::duration>(OsUpDuration());
}

std::chrono::steady_clock::duration OsUpDuration()
{
    return std::chrono::milliseconds(GetTickCount64());
}

bool IsUnix()
{
    return false;
}

bool IsWindows()
{
    return true;
}

std::string OsNativeProductName()
{
    typedef wchar_t*(WINAPI * BrandingFormatStringFunction)(const wchar_t*);
    static auto brandingFormatStringFunction =
        zeus::SharedLibrary::Load(std::string("winbrand.dll"))
            .transform([](const zeus::SharedLibrary& library) { return library.GetTypeSymbol<BrandingFormatStringFunction>("BrandingFormatString"); })
            .value_or(nullptr);
    std::string product;
    if (brandingFormatStringFunction)
    {
        const wchar_t* native = brandingFormatStringFunction(L"%WINDOWS_LONG%");
        if (native)
        {
            product = zeus::CharsetUtils::UnicodeToUTF8(native);
        }
    }
    return product;
}

std::string OsDisplayVersion()
{
    auto version = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kCurrentVersionRegister, true, false, zeus::WinRegistry::ViewType::WIN_KEY_64);
    return version.and_then([](const zeus::WinRegistry& version) { return version.GetStringValue("DisplayVersion"); }).value_or("");
}

std::string OsProductId()
{
    auto version = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kCurrentVersionRegister, true, false, zeus::WinRegistry::ViewType::WIN_KEY_64);
    return version.and_then([](const zeus::WinRegistry& version) { return version.GetStringValue("ProductId"); }).value_or("");
}

std::string OsMachineId()
{
    auto sqm = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kSQMRegister, true, false, zeus::WinRegistry::ViewType::WIN_KEY_64);
    return sqm.and_then([](const zeus::WinRegistry& version) { return version.GetStringValue("MachineId"); }).value_or("");
}

std::chrono::system_clock::time_point OsInstallTime()
{
    auto version = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kCurrentVersionRegister, true, false, zeus::WinRegistry::ViewType::WIN_KEY_64);
    return std::chrono::system_clock::time_point(
        std::chrono::seconds(version.and_then([](const zeus::WinRegistry& version) { return version.GetDwordValue("InstallDate"); }).value_or(0))
    );
}

bool IsWinServer()
{
    const OSVERSIONINFOEXW& vi = GetSystemVersion();
    return vi.wProductType != VER_NT_WORKSTATION;
}

bool IsWin7()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 1 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin8()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 2 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool IsWin8_1()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 3 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool IsWin10()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    //first win10 10.0.10240.16405
    if (10 == version.GetMajor() && version.GetPatch() < 22000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    //first win11 10.0.22000.194
    if (10 == version.GetMajor() && version.GetPatch() >= 22000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin7OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 1;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin8OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 2;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin8_1OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 3;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 22000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2008()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 0 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2008_R2()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 1 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2012()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 2 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2012_R2()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor() && 3 == version.GetMinor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2016()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 14393)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2019()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 17763)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2022()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 20348)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2008OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 0;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2008_R2OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 1;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2012OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 2;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2012_R2OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (6 == version.GetMajor())
    {
        return version.GetMinor() >= 3;
    }
    else if (10 == version.GetMajor())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2016OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 14393)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2019OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 17763)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWinServer2022OrGreater()
{
    DETECTEDT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 20348)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_TH1()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 10240)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_TH2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 10586)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS1()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 14393)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 15063)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS3()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 16299)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS4()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 17134)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS5()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 17763)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_19H1()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 18362)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_19H2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 18363)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_20H1()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 19041)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_20H2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 19042)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_21H1()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 19043)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_21H2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() == 19044)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11_21H2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    //first win11 10.0.22000.194
    if (10 == version.GetMajor() && version.GetPatch() == 22000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11_22H2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    //first win11 10.0.22621.521
    if (10 == version.GetMajor() && version.GetPatch() == 22621)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11_23H2()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    //first win11 10.0.22631.2428
    if (10 == version.GetMajor() && version.GetPatch() == 22631)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_TH1OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 10240)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_TH2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 10586)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS1OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 14393)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 15063)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS3OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 16299)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS4OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 17134)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_RS5OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 17763)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_19H1OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 18362)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_19H2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 18363)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_20H1OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 19041)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_20H2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 19042)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_21H1OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 19043)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin10_21H2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 19044)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11_21H2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 22000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11_22H2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 22621)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsWin11_23H2OrGreater()
{
    DETECTED_NOT_WIN_SERVER;
    auto version = OsVersion();
    if (10 == version.GetMajor() && version.GetPatch() >= 22631)
    {
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace zeus::OS
#endif