#include "zeus/foundation/system/environment.h"
#ifdef _WIN32
#include <memory>
#include <windows.h>
#include <ShlObj.h>
#include "zeus/foundation/system/environment_variable.h"
#include "zeus/foundation/security/win/token.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/system/win/session.h"

namespace fs = std::filesystem;

namespace zeus::Environment
{

size_t ProcessorCount()
{
    auto ProcessorCountFunction = []()
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return si.dwNumberOfProcessors;
    };
    static size_t processorCount = ProcessorCountFunction();
    return processorCount;
}

size_t ProcessorCoreCount()
{
    auto ProcessorCoreCountFunction = []() -> size_t
    {
        size_t cores = 0;
        DWORD  size  = 0;

        GetLogicalProcessorInformation(NULL, &size);
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            return 0;
        }
        const size_t Elements = size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(Elements);
        if (GetLogicalProcessorInformation(&buffer.front(), &size) == FALSE)
        {
            return 0;
        }

        for (size_t i = 0; i < Elements; ++i)
        {
            if (buffer[i].Relationship == RelationProcessorCore)
            {
                ++cores;
            }
        }
        return cores;
    };
    static size_t processorCoreCount = ProcessorCoreCountFunction();
    return processorCoreCount;
}

std::vector<std::string> GetActiveUsers()
{
    std::vector<std::string> users;
    auto                     actives = WinSession::List(WinSession::SessionState::Active);
    users.reserve(actives.size());
    for (const auto& active : actives)
    {
        auto info = WinSession::GetSessionInfo(active.Id());
        if (info.has_value())
        {
            users.emplace_back(info->Username());
        }
    }
    return users;
}

std::string GetHomeString()
{
    return EnvironmentVariable::Get("USERPROFILE").value_or("");
}

std::string GetMachineName()
{
    DWORD size = 0;
    GetComputerNameW(nullptr, &size);
    if (size)
    {
        std::unique_ptr<wchar_t[]> buffer(new wchar_t[size]);
        if (GetComputerNameW(buffer.get(), &size))
        {
            return CharsetUtils::UnicodeToUTF8(buffer.get());
        }
    }
    return "";
}

std::filesystem::path GetDesktopPath()
{
    return std::filesystem::u8path(GetDesktopPathString());
}

std::string GetDesktopPathString()
{
#if _WIN32_WINNT >= 0x0600
    return GetWindowsFolderPathString(FOLDERID_Desktop);
#else
    return GetWindowsFolderPathString(CSIDL_DESKTOPDIRECTORY);
#endif
}

std::filesystem::path GetGlobalAppDataPath()
{
    return fs::u8path(GetGlobalAppDataPathString());
}

std::string GetGlobalAppDataPathString()
{
    return EnvironmentVariable::Get("ProgramData").value_or("");
}

std::filesystem::path GetWindowsFolderPath(const GUID& rfid)
{
    return fs::u8path(GetWindowsFolderPathString(rfid));
}
std::filesystem::path GetWindowsFolderPath(const GUID& rfid, const WinToken& token)
{
    return fs::u8path(GetWindowsFolderPathString(rfid, token));
}
std::filesystem::path GetPublicDesktopPath()
{
    return fs::u8path(GetPublicDesktopPathString());
}

std::string GetWindowsFolderPathString(const GUID& rfid)
{
    std::string path;
    wchar_t*    buffer = nullptr;
    AutoRelease rail(
        [&buffer]()
        {
            if (buffer)
            {
                CoTaskMemFree(buffer);
            }
        }
    );
    SHGetKnownFolderPath(rfid, KF_FLAG_DEFAULT, NULL, &buffer);
    if (buffer)
    {
        path = CharsetUtils::UnicodeToUTF8(buffer);
    }
    return path;
}
std::string GetWindowsFolderPathString(const GUID& rfid, const WinToken& token)
{
    std::string path;
    wchar_t*    buffer = nullptr;
    AutoRelease rail(
        [&buffer]()
        {
            if (buffer)
            {
                CoTaskMemFree(buffer);
            }
        }
    );
    SHGetKnownFolderPath(rfid, KF_FLAG_DEFAULT, token, &buffer);
    if (buffer)
    {
        path = CharsetUtils::UnicodeToUTF8(buffer);
    }
    return path;
}

std::string GetPublicDesktopPathString()
{
#if _WIN32_WINNT >= 0x0600
    return GetWindowsFolderPathString(FOLDERID_PublicDesktop);
#else
    return GetWindowsFolderPathString(CSIDL_COMMON_DESKTOPDIRECTORY);
#endif
}

} // namespace zeus::Environment
#endif
