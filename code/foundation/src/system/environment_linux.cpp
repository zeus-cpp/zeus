#include "zeus/foundation/system/environment.h"
#ifdef __linux__
#include <fstream>
#include <algorithm>
#include <set>
#include <sys/types.h>
#include <utmp.h>
#include <unistd.h>
#include <pwd.h>
#include <fmt/format.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/system/child_process.h"
#include "zeus/foundation/file/kv_file_utils.h"
#include "zeus/foundation/system/linux/xdg_user_dirs.h"

namespace fs = std::filesystem;
using namespace std::string_view_literals;

namespace zeus::Environment
{

size_t ProcessorCount()
{
    auto ProcessorCountFunction = []() -> size_t
    {
        auto processors = GetKVFileValues("/proc/cpuinfo", "processor\t", ":");
        if (processors)
        {
            return processors->size();
        }

        return 0;
    };
    static size_t processorCount = ProcessorCountFunction();
    return processorCount;
}

size_t ProcessorCoreCount()
{
    auto ProcessorCoreCountFunction = []() -> size_t
    {
        auto cores = GetKVFileValue("/proc/cpuinfo", "cpu cores\t", ":");
        if (!cores || cores->empty())
        {
            return ProcessorCount();
        }
        else
        {
            return std::stoul(*cores);
        }
    };
    static size_t processorCoreCount = ProcessorCoreCountFunction();
    return processorCoreCount;
}

std::vector<std::string> GetActiveUsers()
{
    struct utmp              utmp;
    std::vector<std::string> result;
    std::set<std::string>    users;
    std::ifstream            file("/var/run/utmp", std::ios::binary | std::ios::in);
    while (file)
    {
        file.read(reinterpret_cast<char*>(&utmp), sizeof(utmp));
        if (utmp.ut_type == USER_PROCESS)
        {
            users.emplace(utmp.ut_user);
        }
    }
    result.reserve(users.size());
    for (const auto& user : users)
    {
        result.emplace_back(user);
    }
    return result;
}

std::string GetHomeString()
{
    std::string    path;
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
    {
        path = pwd->pw_dir;
    }
    else
    {
        pwd = getpwuid(geteuid());
        if (pwd)
        {
            path = pwd->pw_dir;
        }
    }
    return path;
}

std::string GetMachineName()
{
    return Trim(FileContent(std::filesystem::path("/proc/sys/kernel/hostname")).value_or(""));
}

std::filesystem::path GetDesktopPath()
{
    return GetXdgUserDir(XdgUserDirName::DESKTOP);
}

std::string GetDesktopPathString()
{
    return GetDesktopPath().u8string();
}

std::filesystem::path GetGlobalAppDataPath()
{
    static std::filesystem::path appDataPath(GetGlobalAppDataPathString());
    return appDataPath;
}

std::string GetGlobalAppDataPathString()
{
    static std::string appDataPath("/var/tmp/");
    return appDataPath;
}

zeus::expected<uid_t, std::error_code> GetUserId(const std::string& username)
{
    struct passwd* pwd = getpwnam(username.c_str());
    if (pwd)
    {
        return pwd->pw_uid;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<std::string, std::error_code> GetUsername(uid_t uid)
{
    struct passwd* pwd = getpwuid(uid);
    if (pwd)
    {
        return pwd->pw_name;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<std::filesystem::path, std::error_code> GetUserHome(uid_t uid)
{
    std::string    path;
    struct passwd* pwd = getpwuid(uid);
    if (pwd)
    {
        return pwd->pw_dir;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<std::filesystem::path, std::error_code> GetUserHome(const std::string& username)
{
    struct passwd* pwd = getpwnam(username.c_str());
    if (pwd)
    {
        return pwd->pw_dir;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

std::optional<std::map<std::string, std::string>> GetActiveX11Environment()
{
    static const std::vector<std::string> X11EnvironmentKey = {"DISPLAY", "XAUTHORITY"};
    auto                                  displayManager    = FileContent(fs::path("/etc/X11/default-display-manager"));
    if (!displayManager.has_value())
    {
        return std::nullopt;
    }
    fs::path displayManagerPath(Trim(displayManager.value()));
    if (displayManagerPath.empty())
    {
        return std::nullopt;
    }
    std::set<Process::PID> displayManagerList;
    auto                   allProcess = Process::ListAll();
    for (const auto& process : allProcess)
    {
        if (process.ExePath() == displayManagerPath)
        {
            displayManagerList.emplace(process.Id());
        }
    }
    if (displayManagerList.empty())
    {
        std::string displayManagerFilename = displayManagerPath.filename().string();
        for (const auto& process : allProcess)
        {
            if (process.Name() == displayManagerFilename)
            {
                displayManagerList.emplace(process.Id());
            }
        }
    }
    if (displayManagerList.empty())
    {
        return std::nullopt;
    }
    std::set<Process::PID> launchers;
    for (const auto& process : allProcess)
    {
        if (0 == process.UserId())
        {
            continue;
        }
        if (std::any_of(displayManagerList.begin(), displayManagerList.end(), [&](Process::PID pid) { return process.ParentId() == pid; }))
        {
            launchers.emplace(process.Id());
        }
    }
    if (launchers.empty())
    {
        return std::nullopt;
    }
    for (const auto& launcher : launchers)
    {
        auto env = Process::GetProcessEnvironmentVariable(launcher);
        if (!env.has_value())
        {
            continue;
        }
        std::map<std::string, std::string> result;
        for (const auto& key : X11EnvironmentKey)
        {
            auto value = env->find(key);
            if (value != env->end())
            {
                result[key] = value->second;
            }
        }
        if (result.empty())
        {
            continue;
        }
        for (auto& [key, value] : env.value())
        {
            if (StartWith(key, "XDG_"))
            {
                result[key] = value;
            }
        }
        return result;
    }
    return std::nullopt;
}

bool IsService(Process::PID pid)
{
    fs::path    cgroup(fmt::format("/proc/{}/cgroup", pid));
    std::string serviceName;
    FileEachLine(
        cgroup,
        [&serviceName](const std::string_view& line)
        {
            if (EndWith(line, ".service"sv))
            {
                if (auto pos = line.rfind('/'); pos != std::string_view::npos)
                {
                    serviceName = std::string(line.substr(pos + 1));
                    return serviceName.empty();
                }
            }
            return true;
        }
    );
    if (serviceName.empty())
    {
        return false;
    }
    auto child = ChildProcessExecutor().EnableRedirectStdout(true).ExecuteCommand(fmt::format("systemctl show {} -p MainPID --value", serviceName));
    if (child.has_value() && child->HasStdout())
    {
        std::string  out;
        Process::PID mainPid = {};
        child->GetStdout() >> out;
        child->Kill();
        try
        {
            mainPid = std::stoul(out);
        }
        catch (const std::exception& e)
        {
            return false;
        }
        if (mainPid == pid)
        {
            return true;
        }
    }
    return false;
}
} // namespace zeus::Environment
#endif
