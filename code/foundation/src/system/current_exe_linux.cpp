#include "zeus/foundation/system/current_exe.h"
#ifdef __linux__
#include <filesystem>
#include <memory>
#include <cassert>
#include <dlfcn.h>
#include <unistd.h>
#include <pwd.h>
#include "zeus/foundation/system/environment.h"

namespace fs = std::filesystem;

namespace
{
std::string GetExePath()
{
    fs::path        link("/proc/self/exe");
    std::error_code ec;
    return fs::read_symlink(link, ec).string();
}

std::string GetAddressModulePath(void* address = nullptr)
{
    std::string path;
    if (!address)
    {
        address = reinterpret_cast<void*>(&GetAddressModulePath);
    }
    Dl_info info {};
    if (dladdr(address, &info))
    {
        path = std::string(info.dli_fname);
    }
    return path;
}
} // namespace

namespace zeus::CurrentExe
{
bool IsService()
{
    return Environment::IsService(GetProcessId());
}

std::string GetUsername()
{
    std::string    user;
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
    {
        user = pwd->pw_name;
    }
    return user;
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

uid_t GetUserId()
{
    return getuid();
}

gid_t GetGroupId()
{
    return getgid();
}

uid_t GetEffectiveUserId()
{
    return geteuid();
}

gid_t GetEffectiveGroupId()
{
    return getegid();
}

bool HasRootPermission()
{
    return 0 == geteuid();
}

std::string GetEffectiveUsername()
{
    std::string    user;
    struct passwd* pwd = getpwuid(geteuid());
    if (pwd)
    {
        user = pwd->pw_name;
    }
    return user;
}

} // namespace zeus::CurrentExe
#endif //  __linux__
