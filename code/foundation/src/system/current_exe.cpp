#include "zeus/foundation/system/current_exe.h"

namespace fs = std::filesystem;

namespace zeus::CurrentExe
{

Process::PID GetProcessId()
{
    return Process::GetCurrentId();
}

std::string GetAppName()
{
    static std::string appName(GetAppPath().stem().u8string());
    return appName;
}

std::filesystem::path GetAppPath()
{
    static fs::path appPath(fs::u8path(GetAppPathString()));
    return appPath;
}

std::filesystem::path GetAppDir()
{
    static fs::path appDir(GetAppPath().parent_path());
    return appDir;
}

std::string GetAppDirString()
{
    static std::string appDir(GetAppDir().u8string());
    return appDir;
}

std::filesystem::path GetModulePath(void* address)
{
    return fs::u8path(GetModulePathString(address));
}

std::filesystem::path GetModuleDir(void* address)
{
    return GetModulePath(address).parent_path();
}

std::string GetModuleDirString(void* address)
{
    return GetModuleDir(address).u8string();
}

} // namespace zeus::CurrentExe
