#pragma once
#include <string>
#include <set>
#include <map>
#include <memory>
#include <chrono>
#include <optional>
#include <filesystem>
#include <vector>
#include <string_view>
#include <system_error>
#ifdef __linux__
#include <sys/types.h>
#endif
#include <zeus/expected.hpp>
#ifdef _WIN32
#include "zeus/foundation/system/win/session.h"
#endif
namespace zeus
{
struct ProcessImpl;
class Process
{
public:
#ifdef _WIN32
    using PID = unsigned long;
#else
    using PID = pid_t;
#endif

    Process(const Process& other);
    Process(Process&& other) noexcept;
    ~Process();
    Process& operator=(const Process& other);
    Process& operator=(Process&& other) noexcept;
    PID      Id() const;
    std::string Name() const; //注意，linux的进程名是指/proc/pid/status中的Name字段，其并不是一定和可执行文件的文件名相同
    std::filesystem::path                 ExePath() const;
    std::string                           ExePathString() const;
    PID                                   ParentId() const;
    std::chrono::system_clock::time_point CreateTime() const;
    uint64_t                              CreateTimeTick() const;
#ifdef _WIN32
    WinSession::SessionId SessionId() const;
#endif
#ifdef __linux__
    uid_t UserId() const;
    gid_t GroupId() const;
    uid_t EffectiveUserId() const;
    gid_t EffectiveGroupId() const;
#endif
    bool operator<(const Process& other) const;
public:

    static PID                                   GetCurrentId();
    static Process                               GetCurrentProcess();
    static std::set<Process>                     FindProcessByPath(std::string_view path, bool ignoreCase = true);
    static std::set<Process>                     FindProcessByName(std::string_view name, bool ignoreCase = true);
    static std::optional<Process>                FindProcessById(const PID& pid);
    static std::set<PID>                         ListId();
    static std::set<Process>                     ListAll();
    static std::map<PID, Process>                ListMap();
    static zeus::expected<void, std::error_code> KillProcess(
        const PID& pid, const std::chrono::steady_clock::duration& timeout = std::chrono::milliseconds(0)
    );
    static zeus::expected<std::filesystem::path, std::error_code>    GetProcessExePath(const PID& pid);
    static zeus::expected<std::string, std::error_code>              GetProcessExePathString(const PID& pid);
    static zeus::expected<std::vector<std::string>, std::error_code> GetProcessCmdlineArguments(const PID& pid);
#ifdef __linux__
    static zeus::expected<bool, std::error_code>                                  IsProcessZombie(const PID& pid);
    static zeus::expected<std::map<std::string, std::string>, std::error_code>    GetProcessEnvironmentVariable(const PID& pid);
    static zeus::expected<uint64_t, std::error_code>                              GetProcessCreateTimeTick(const PID& pid);
    static zeus::expected<std::chrono::system_clock::time_point, std::error_code> GetProcessCreateTime(const PID& pid);
#endif
    static std::optional<std::vector<std::string>> SplitCmdline(const std::string& cmdline);
protected:
    Process();
private:
    std::unique_ptr<ProcessImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
