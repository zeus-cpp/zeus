#include "zeus/foundation/system/current_exe.h"
#ifdef _WIN32
#include <memory>
#include <Windows.h>
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/system/environment_variable.h"
#include "zeus/foundation/system/win/file_version_info.h"

namespace fs = std::filesystem;

namespace
{

std::string GetModulePath(HMODULE module)
{
    uint32_t                   len = 1024;
    std::unique_ptr<wchar_t[]> buffer(std::make_unique<wchar_t[]>(len));
    GetModuleFileNameW(module, buffer.get(), len);
    while (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        len *= 2;
        buffer = std::unique_ptr<wchar_t[]>(std::make_unique<wchar_t[]>(len));
        GetModuleFileNameW(module, buffer.get(), len);
    }
    return zeus::CharsetUtils::UnicodeToUTF8(buffer.get());
}

std::string GetExePath()
{
    return GetModulePath(nullptr);
}

std::string GetAddressModulePath(void* address = nullptr)
{
    std::string path;
    HMODULE     module;
    if (!address)
    {
        address = &GetAddressModulePath;
    }
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<char*>(address), &module))
    {
        path = GetModulePath(module);
    }
    return path;
}

std::pair<zeus::Version, zeus::Version> GetAppAllVersion()
{
    auto fileVersion = zeus::WinFileVersionInfo::Load(zeus::CurrentExe::GetAppPath());
    if (fileVersion.has_value())
    {
        return std::make_pair(fileVersion->GetFileVersion().value_or(zeus::Version()), fileVersion->GetProductVersion().value_or(zeus::Version()));
    }
    else
    {
        return std::make_pair(zeus::Version(), zeus::Version());
    }
}

std::pair<zeus::Version, zeus::Version>& GetAppVersionCache()
{
    static std::pair<zeus::Version, zeus::Version> cache = GetAppAllVersion();
    return cache;
}

} // namespace

namespace zeus::CurrentExe
{

bool IsService()
{
    auto parentProcess = Process::FindProcessById(Process::GetCurrentProcess().ParentId());
    return (
        IEqual(parentProcess->Name(), "services.exe") && IEqual(parentProcess->ExePathString(), R"(C:\WINDOWS\system32\services.exe)") &&
        0 == GetProcessSessionId() && GetUsername() == "SYSTEM"
    );
}

std::string GetUsername()
{
    DWORD                      bufferLength = 100;
    std::unique_ptr<wchar_t[]> buffer(std::make_unique<wchar_t[]>(bufferLength));
    std::memset(buffer.get(), 0, bufferLength);
    GetUserNameW(buffer.get(), &bufferLength);
    return CharsetUtils::UnicodeToUTF8(buffer.get());
}

std::string GetAppPathString()
{
    static std::string appPath(GetExePath());
    return appPath;
}

std::string GetModulePathString(void* address)
{
    return GetAddressModulePath(address);
}

std::filesystem::path GetProgramDataDir()
{
    return fs::u8path(EnvironmentVariable::Get("Programdata").value_or(""));
}

std::filesystem::path GetAppDataDir()
{
    return fs::u8path(EnvironmentVariable::Get("APPDATA").value_or(""));
}

std::string GetProgramDataDirString()
{
    return EnvironmentVariable::Get("Programdata").value_or("");
}

std::string GetAppDataDirString()
{
    return EnvironmentVariable::Get("APPDATA").value_or("");
}

size_t GetProcessSessionId()
{
    DWORD session = 0;
    ProcessIdToSessionId(GetProcessId(), &session);
    return session;
}

zeus::Version GetFileVersion()
{
    return GetAppVersionCache().first;
}

zeus::Version GetProductVersion()
{
    return GetAppVersionCache().second;
}

zeus::Version GetModuleFileVersion(void* address)
{
    auto fileVersion = zeus::WinFileVersionInfo::Load(zeus::CurrentExe::GetModulePath(address));
    if (fileVersion.has_value())
    {
        return fileVersion->GetFileVersion().value_or(zeus::Version());
    }
    else
    {
        return zeus::Version {};
    }
}

zeus::Version GetModuleProductVersion(void* address)
{
    auto fileVersion = zeus::WinFileVersionInfo::Load(zeus::CurrentExe::GetModulePath(address));
    if (fileVersion.has_value())
    {
        return fileVersion->GetProductVersion().value_or(zeus::Version());
    }
    else
    {
        return zeus::Version {};
    }
}

bool IsWow64()
{
    BOOL wow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &wow64);
    return wow64;
}

} // namespace zeus::CurrentExe
#endif //
