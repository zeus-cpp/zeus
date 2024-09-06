#include "zeus/foundation/system/process.h"

#ifdef __linux__
#include <functional>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include "zeus/foundation/file/kv_file_utils.h"
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/byte/byte_utils.h"
#include "zeus/foundation/system/os.h"
#include "zeus/foundation/system/environment_variable.h"
#include "zeus/foundation/time/time.h"
#include "zeus/foundation/core/system_error.h"
#include "impl/process_impl.h"

namespace fs = std::filesystem;

namespace zeus
{

namespace
{

const std::string kStatusName     = "Name";
const std::string kStatusParendId = "PPid";
const std::string kStatusUId      = "Uid";
const std::string kStatusGId      = "Gid";

const std::string kStatusFilename = "status";
const std::string kStatusDelim    = ":\t";

zeus::expected<std::filesystem::path, std::error_code> GetFullExePath(Process::PID pid)
{
    fs::path link("/proc");
    link /= std::to_string(pid);
    link /= "exe";
    std::error_code ec;
    auto            exe = fs::read_symlink(link, ec);
    if (ec)
    {
        return zeus::unexpected(ec);
    }
    return std::move(exe);
}

fs::path GetFullExePath(const fs::path& processDir)
{
    fs::path        link = processDir / "exe";
    std::error_code ec;
    return fs::read_symlink(link, ec);
}

zeus::expected<bool, std::error_code> IsZombie(const fs::path& processDir)
{
    auto content = FileContent(processDir / "stat");
    if (!content)
    {
        return zeus::unexpected(content.error());
    }
    if (!content->empty())
    {
        if (auto iter = content->find_last_of(')'); iter != std::string::npos)
        {
            char state = 0;
            std::sscanf(content->substr(iter + 1).c_str(), " %c", &state);
            return 'Z' == state;
        }
    }
    return true;
}

zeus::expected<void, std::error_code> WaitProcessExit(const fs::path& processDir, const std::chrono::steady_clock::duration& timeout)
{
    std::chrono::microseconds           period(100);
    std::chrono::steady_clock::duration duration(0);
    while (fs::exists(processDir) && !IsZombie(processDir))
    {
        if (duration >= timeout)
        {
            return zeus::unexpected(SystemError {ETIMEDOUT});
        }
        Sleep(period);
        duration += period;
    }
    return {};
}

void EnumProcess(const std::function<bool(const fs::path& processDir, Process::PID pid)>& handler)
{
    std::error_code ec;
    for (const auto& item : fs::directory_iterator("/proc", ec))
    {
        if (!item.is_directory(ec))
        {
            continue;
        }
        auto filename = item.path().filename().string();
        if (zeus::IsNumber(filename) && !(IsZombie(item.path()).value_or(true)))
        {
            auto pid = std::stol(filename);
            if (!handler(item.path(), pid))
            {
                return;
            }
        }
    }
}

Process::PID GetStatusPPID(const std::map<std::string, std::string>& status)
{
    auto iter = status.find(kStatusParendId);
    if (iter != status.end())
    {
        return std::stol(iter->second);
    }
    return 0;
}

std::string GetProcessName(const fs::path& /*processDir*/, const std::map<std::string, std::string>& status)
{
    std::string processName;
    auto        iter = status.find(kStatusName);
    if (iter != status.end())
    {
        processName = iter->second;
    }
    return processName;
}

zeus::expected<uint64_t, std::error_code> GetCreateTimeTick(const fs::path& processDir)
{
    auto content = FileContent(processDir / "stat");
    if (!content.has_value())
    {
        return zeus::unexpected(content.error());
    }
    if (auto iter = content->find_last_of(')'); iter != std::string::npos)
    {
        content = content->substr(iter + 1);
    }
    unsigned long long starttime = 0;
    //starttime is the 22th value in /proc/pid/stat
    std::sscanf(content->c_str(), " %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %llu", &starttime);
    return starttime;
}

std::chrono::system_clock::time_point CastCreateTime(uint64_t startTick)
{
    auto tickPerSecond = sysconf(_SC_CLK_TCK);
    return OS::OsBootTime() + std::chrono::seconds(startTick / tickPerSecond);
}

std::tuple<unsigned int, unsigned int> GetIds(const std::map<std::string, std::string>& status, const std::string& key)
{
    auto iter = status.find(key);
    if (iter != status.end())
    {
        auto ids = Split(iter->second, "\t");
        if (ids.size() >= 2)
        {
            return {std::stoul(ids.at(0)), std::stoul(ids.at(1))};
        }
    }
    return {0, 0};
}

void FillProcessImpl(ProcessImpl& impl, const fs::path& processDir, Process::PID pid, const std::map<std::string, std::string>& status)
{
    impl.pid = pid;
    if (impl.exePath.empty())
    {
        impl.exePath = GetFullExePath(processDir);
    }
    if (impl.exePathString.empty())
    {
        impl.exePathString = impl.exePath.string();
    }
    if (impl.name.empty())
    {
        impl.name = GetProcessName(processDir, status);
    }
    impl.ppid                                     = GetStatusPPID(status);
    impl.createTimeTick                           = GetCreateTimeTick(processDir).value_or(0);
    impl.createTime                               = CastCreateTime(impl.createTimeTick);
    std::tie(impl.userId, impl.effectiveUserId)   = GetIds(status, kStatusUId);
    std::tie(impl.groupId, impl.effectiveGroupId) = GetIds(status, kStatusGId);
}

void FillProcessImpl(ProcessImpl& impl, const fs::path& processDir, Process::PID pid)
{
    auto status = PickKVFileUniqueData(processDir / kStatusFilename, {kStatusName, kStatusParendId, kStatusUId, kStatusGId}, kStatusDelim);
    if (status)
    {
        FillProcessImpl(impl, processDir, pid, *status);
    }
}

} // namespace

Process::PID Process::GetCurrentId()
{
    return ::getpid();
}
Process Process::GetCurrentProcess()
{
    Process process;
    FillProcessImpl(*process._impl, "/proc/self", ::getpid());
    return process;
}
std::set<Process> Process::FindProcessByPath(std::string_view path, bool ignoreCase)
{
    std::set<Process> processes;
    EnumProcess(
        [&processes, expectPath = path, ignoreCase](const fs::path& processDir, Process::PID pid)
        {
            auto path       = GetFullExePath(processDir);
            auto pathString = path.string();
            if (ignoreCase ? IEqual(expectPath, pathString) : expectPath == pathString)
            {
                Process process;
                process._impl->exePath       = path;
                process._impl->exePathString = pathString;
                FillProcessImpl(*process._impl, processDir, pid);
                processes.emplace(std::move(process));
            }
            return true;
        }
    );
    return processes;
}
std::set<Process> Process::FindProcessByName(std::string_view name, bool ignoreCase)
{
    std::set<Process> processes;
    EnumProcess(
        [&processes, expectName = name, ignoreCase](const fs::path& processDir, Process::PID pid)
        {
            auto status = PickKVFileUniqueData(processDir / kStatusFilename, {kStatusName, kStatusParendId, kStatusUId, kStatusGId}, kStatusDelim);
            if (status)
            {
                auto name = GetProcessName(processDir, *status);
                if (ignoreCase ? IEqual(expectName, name) : expectName == name)
                {
                    Process process;
                    process._impl->name = name;
                    FillProcessImpl(*process._impl, processDir, pid, *status);
                    processes.emplace(std::move(process));
                }
            }
            return true;
        }
    );
    return processes;
}
std::optional<Process> Process::FindProcessById(const PID& pid)
{
    std::optional<Process> processes;
    std::error_code        ec;
    auto                   processDir = fs::path("/proc") / std::to_string(pid);
    if (fs::exists(processDir, ec) && !IsZombie(processDir).value_or(true))
    {
        Process process;
        FillProcessImpl(*process._impl, processDir, pid);
        processes.emplace(std::move(process));
    }
    return processes;
}
std::set<Process::PID> Process::ListId()
{
    std::set<Process::PID> processes;
    EnumProcess(
        [&processes](const fs::path& /*processDir*/, Process::PID pid)
        {
            processes.emplace(pid);
            return true;
        }
    );
    return processes;
}
std::set<Process> Process::ListAll()
{
    std::set<Process> processes;
    EnumProcess(
        [&processes](const fs::path& processDir, Process::PID pid)
        {
            Process process;
            FillProcessImpl(*process._impl, processDir, pid);
            processes.emplace(std::move(process));
            return true;
        }
    );
    return processes;
}

std::map<Process::PID, Process> Process::ListMap()
{
    std::map<PID, Process> processes;
    EnumProcess(
        [&processes](const fs::path& processDir, Process::PID pid)
        {
            Process process;
            FillProcessImpl(*process._impl, processDir, pid);
            processes.emplace(pid, std::move(process));
            return true;
        }
    );
    return processes;
}

zeus::expected<void, std::error_code> Process::KillProcess(const PID& pid, const std::chrono::steady_clock::duration& timeout)
{
    if (0 == kill(pid, SIGKILL))
    {
        fs::path        processDir("/proc");
        std::error_code ec;
        processDir /= std::to_string(pid);
        return WaitProcessExit(processDir, timeout);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<std::filesystem::path, std::error_code> Process::GetProcessExePath(const PID& pid)
{
    return GetFullExePath(pid);
}
zeus::expected<std::string, std::error_code> Process::GetProcessExePathString(const PID& pid)
{
    auto exe = GetFullExePath(pid);
    if (exe.has_value())
    {
        return exe->string();
    }
    return zeus::unexpected(exe.error());
}

zeus::expected<std::vector<std::string>, std::error_code> Process::GetProcessCmdlineArguments(const PID& pid)
{
    using namespace std::literals;
    return FileContent(std::string_view("/proc/"s + std::to_string(pid) + "/cmdline"), true)
        .and_then(
            [](std::string const& content) -> zeus::expected<std::vector<std::string>, std::error_code>
            {
                if (!content.empty())
                {
                    auto split = ByteSplit(
                        {reinterpret_cast<const uint8_t*>(content.data()), content.size()}, {reinterpret_cast<const uint8_t*>("\0"), sizeof(char)},
                        sizeof(char)
                    );
                    std::vector<std::string> args;
                    for (const auto& item : split)
                    {
                        args.emplace_back(std::string(reinterpret_cast<const char*>(item.Data()), item.Size()));
                    }
                    return std::move(args);
                }
                else
                {
                    return {};
                }
            }
        );
}

zeus::expected<bool, std::error_code> Process::IsProcessZombie(const PID& pid)
{
    return IsZombie(fs::path("/proc") / std::to_string(pid));
}

zeus::expected<std::map<std::string, std::string>, std::error_code> Process::GetProcessEnvironmentVariable(const PID& pid)
{
    using namespace std::literals;
    return FileContent(std::string_view("/proc/"s + std::to_string(pid) + "/environ"), true)
        .and_then(
            [](std::string const& content) -> zeus::expected<std::map<std::string, std::string>, std::error_code>
            {
                if (!content.empty())
                {
                    return EnvironmentVariable::ParseEnvironmentVariableData(content.data(), content.size());
                }
                return {};
            }
        );
}

zeus::expected<uint64_t, std::error_code> Process::GetProcessCreateTimeTick(const PID& pid)
{
    return GetCreateTimeTick(fs::path("/proc") / std::to_string(pid));
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> Process::GetProcessCreateTime(const PID& pid)
{
    auto tick = GetProcessCreateTimeTick(pid);
    if (tick.has_value())
    {
        return CastCreateTime(tick.value());
    }
    return zeus::unexpected(tick.error());
}
} // namespace zeus

#endif
