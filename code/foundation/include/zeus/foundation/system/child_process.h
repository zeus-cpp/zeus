#pragma once
#include <vector>
#include <utility>
#include <cstdint>
#include <chrono>
#include <memory>
#include <system_error>
#include <filesystem>
#include <string_view>
#ifdef __linux__
#include <unistd.h>
#endif
#include <zeus/expected.hpp>
#include <zeus/foundation/system/process.h>
#ifdef _WIN32
#include "zeus/foundation/core/win/win_windef.h"
#endif

namespace zeus
{

struct ChildProcessImpl;
class ChildProcess
{
public:
    friend class ChildProcessExecutor;
    ChildProcess();
    ~ChildProcess();
    ChildProcess(ChildProcess const&)            = delete;
    ChildProcess& operator=(ChildProcess const&) = delete;
    ChildProcess(ChildProcess&& other) noexcept;
    ChildProcess& operator=(ChildProcess&&) = delete;

    /// <summary>
    /// 等待执行结束
    /// </summary>
    /// <param name="timeout"></param>
    /// <returns>执行返回值</returns>
    zeus::expected<int32_t, std::error_code> Wait(const std::chrono::steady_clock::duration& timeout);
    zeus::Process::PID                       GetPID();
#ifdef _WIN32
    zeus::expected<void, std::error_code> Kill(uint32_t code = 0);
#endif
#ifdef __linux__
    zeus::expected<void, std::error_code> Kill();
#endif
    bool          HasStdout();
    bool          HasStderr();
    bool          HasStdin();
    //如开启重定向后不读取对应的输出，可能导致子进程输出缓冲满后阻塞
    std::istream& GetStdout();
    std::istream& GetStderr();
    std::ostream& GetStdin();
#ifdef _WIN32
#endif
private:
    std::unique_ptr<ChildProcessImpl> _impl;
};

struct ChildProcessExecutorImpl;
class ChildProcessExecutor
{
public:
    ChildProcessExecutor();
    ~ChildProcessExecutor();
    ChildProcessExecutor(const ChildProcessExecutor&)            = delete;
    ChildProcessExecutor& operator=(const ChildProcessExecutor&) = delete;
    ChildProcessExecutor(ChildProcessExecutor&&)                 = delete;
    ChildProcessExecutor& operator=(ChildProcessExecutor&&)      = delete;
    ChildProcessExecutor& EnableRedirectStdout(bool redirect);
    ChildProcessExecutor& EnableRedirectStderr(bool redirect);
    ChildProcessExecutor& EnableRedirectStdin(bool redirect);
    ChildProcessExecutor& SetEnvironment(std::string_view name, std::string_view value);
    ChildProcessExecutor& SetWorkingDirectory(std::string_view path);
    ChildProcessExecutor& SetWorkingDirectory(const std::filesystem::path& path);
#ifdef _WIN32
    ChildProcessExecutor& EnableShow(bool show);
    ChildProcessExecutor& SetUseToken(HANDLE token, bool useEnv);
    ChildProcessExecutor& AddInheritedHandle(HANDLE handle);
#endif
#ifdef __linux__
    ChildProcessExecutor& AddInheritedFd(int fd);
    ChildProcessExecutor& DisableStdout();
    ChildProcessExecutor& DisableStderr();
    ChildProcessExecutor& DisableStdin();
    ChildProcessExecutor& SetUID(uid_t uid);
    ChildProcessExecutor& SetGID(gid_t gid);
#endif
    /// <summary>
    /// 执行shell脚本，无法识别重定向、管道等操作符，可以自动对空格参数加引号
    /// windows-bat
    /// linux-未实现
    /// </summary>
    /// <param name="script">脚本名</param>
    /// <param name="args">参数列表</param>
    /// <returns>创建进程结果，脚本执行体</returns>
    zeus::expected<ChildProcess, std::error_code> ExecuteScript(std::string_view script, const std::vector<std::string>& args);
    /// <summary>
    /// 执行shell命令，不会自动对空格参数加引号，直接转发给shell执行
    /// windows-cmd
    /// linux-未实现
    /// </summary>
    /// <param name="cmd">命令名</param>
    /// <returns>创建进程结果，命令执行体</returns>
    zeus::expected<ChildProcess, std::error_code> ExecuteCommand(std::string_view cmd);
    /// <summary>
    /// 执行进程
    /// windows-bat
    /// linux-未实现
    /// </summary>
    /// <param name="process">进程名</param>
    /// <param name="args">参数列表</param>
    /// <returns>创建进程结果，进程执行体</returns>
    zeus::expected<ChildProcess, std::error_code> ExecuteProcess(std::string_view process, const std::vector<std::string>& args);
private:
    std::unique_ptr<ChildProcessExecutorImpl> _impl;
};

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
