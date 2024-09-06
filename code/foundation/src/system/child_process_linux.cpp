#include "zeus/foundation/system/child_process.h"
#ifdef __linux__
#include <filesystem>
#include <optional>
#include <array>
#include <list>
#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include "zeus/foundation/resource/pipe_stream_buffer.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/system/environment_variable.h"
#include "zeus/foundation/system/process.h"
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/core/posix/eintr_wrapper.h"
#include "zeus/foundation/container/container_cast.hpp"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/time/time.h"
#include "impl/child_process_impl.h"

namespace fs = std::filesystem;

namespace zeus
{
struct ChildProcessImpl
{
    zeus::Process::PID              pid = 0;
    LinuxFileDescriptor             stdoutReadFd;
    LinuxFileDescriptor             stderrReadFd;
    LinuxFileDescriptor             stdinWriteFd;
    std::optional<PipeStreamBuffer> stdoutReadBuffer;
    std::optional<PipeStreamBuffer> stderrReadBuffer;
    std::optional<PipeStreamBuffer> stdinWriteBuffer;
    std::optional<std::istream>     stdoutReadStream;
    std::optional<std::istream>     stderrReadStream;
    std::optional<std::ostream>     stdinWriteStream;
};

namespace
{

std::vector<std::string> CreateLinuxEnvironmentArray(const std::map<std::string, std::string>& environmentVariables)
{
    std::vector<std::string> result;
    for (auto& [key, value] : environmentVariables)
    {
        result.emplace_back(key + "=" + value);
    }
    return result;
}

zeus::expected<std::unique_ptr<ChildProcessImpl>, std::error_code> CreateLinuxProcess(
    const std::string& process, const std::vector<std::string>& args, const std::map<std::string, std::string>& environmentVariables,
    const std::string& workDir, const std::list<int>& inheritedFds, bool redirectStdout, bool redirectStderr, bool redirectStdin,
    std::optional<uid_t> uid, std::optional<gid_t> gid, bool disableStdout, bool disableStderr, bool disableStdin
)
{
    std::vector<char*> cmds(args.size() + 2);
    auto               processName = fs::path(process).filename().string();
    cmds.front()                   = processName.data();
    for (size_t index = 0; index < args.size(); ++index)
    {
        cmds.at(index + 1) = const_cast<char*>(args.at(index).data());
    }
    cmds.back() = nullptr;

    std::vector<char*>       envData;
    std::vector<std::string> environmentVariableList;
    if (!environmentVariables.empty())
    {
        auto oldEnvData = EnvironmentVariable::ParseEnvironmentVariableArray(::environ);
        for (const auto& [key, value] : environmentVariables)
        {
            oldEnvData[key] = value;
        }
        environmentVariableList = CreateLinuxEnvironmentArray(oldEnvData);
        envData.reserve(environmentVariableList.size() + 1);
        for (const auto& item : environmentVariableList)
        {
            envData.emplace_back(const_cast<char*>(item.data()));
        }
    }
    else
    {
        for (char** env = ::environ; *env; ++env)
        {
            envData.emplace_back(*env);
        }
    }
    envData.emplace_back(nullptr);

    std::set<int>                     inheritFds = ListToSet(inheritedFds);
    std::optional<std::array<int, 2>> stdoutPipe;
    std::optional<std::array<int, 2>> stderrPipe;
    std::optional<std::array<int, 2>> stdinPipe;
    std::array<int, 2>                execEventPipe = {};
    {
        int result = pipe2(execEventPipe.data(), O_CLOEXEC);
        if (result < 0)
        {
            return zeus::unexpected(GetLastSystemError());
        }
        inheritFds.emplace(execEventPipe.at(0));
        inheritFds.emplace(execEventPipe.at(1));
    }
    if (redirectStdout && !disableStdout)
    {
        stdoutPipe.emplace();
        int result = pipe(stdoutPipe->data());
        if (result < 0)
        {
            return zeus::unexpected(GetLastSystemError());
        }
        inheritFds.emplace(stdoutPipe->at(0));
        inheritFds.emplace(stdoutPipe->at(1));
    }
    if (redirectStderr && !disableStdout)
    {
        stderrPipe.emplace();
        int result = pipe(stderrPipe->data());
        if (result < 0)
        {
            return zeus::unexpected(GetLastSystemError());
        }
        inheritFds.emplace(stderrPipe->at(0));
        inheritFds.emplace(stderrPipe->at(1));
    }
    if (redirectStdin && !disableStdin)
    {
        stdinPipe.emplace();
        int result = pipe(stdinPipe->data());
        if (result < 0)
        {
            return zeus::unexpected(GetLastSystemError());
        }
        inheritFds.emplace(stdinPipe->at(1));
        inheritFds.emplace(stdinPipe->at(0));
    }
    long maxFd = 0;
    {
        DIR* dir = opendir("/proc/self/fd");
        for (const dirent* entry = readdir(dir); entry; entry = readdir(dir))
        {
            if (DT_DIR == entry->d_type && IsNumber(entry->d_name))
            {
                auto fd = std::stol(entry->d_name);
                if (fd > maxFd)
                {
                    maxFd = fd;
                }
            }
        }
        closedir(dir);
    }

    auto pid = fork();
    if (pid < 0)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    else if (0 == pid)
    {
        maxFd += 256;
        for (int fd = 3; fd < maxFd; ++fd)
        {
            if (!inheritFds.count(fd))
            {
                HANDLE_EINTR(close(fd));
            }
        }

        if (disableStdout)
        {
            HANDLE_EINTR(close(STDOUT_FILENO));
        }
        if (disableStderr)
        {
            HANDLE_EINTR(close(STDERR_FILENO));
        }
        if (disableStdin)
        {
            HANDLE_EINTR(close(STDIN_FILENO));
        }
        HANDLE_EINTR(close(execEventPipe.at(0)));
        if (stdoutPipe.has_value())
        {
            HANDLE_EINTR(dup2(stdoutPipe->at(1), STDOUT_FILENO));
            HANDLE_EINTR(close(stdoutPipe->at(0)));
        }
        if (stderrPipe.has_value())
        {
            HANDLE_EINTR(dup2(stderrPipe->at(1), STDERR_FILENO));
            HANDLE_EINTR(close(stderrPipe->at(0)));
        }
        if (stdinPipe.has_value())
        {
            HANDLE_EINTR(dup2(stdinPipe->at(0), STDIN_FILENO));
            HANDLE_EINTR(close(stdinPipe->at(1)));
        }

        if (gid.has_value())
        {
            setgid(*gid);
        }
        if (uid.has_value())
        {
            setuid(*uid);
        }
        if (!workDir.empty())
        {
            chdir(workDir.c_str());
        }
        auto result = ::execve(process.c_str(), cmds.data(), envData.data());
        if (result < 0)
        {
            int err = errno;
            HANDLE_EINTR(write(execEventPipe.at(1), &err, sizeof(err)));
            exit(0);
        }
        return zeus::unexpected(GetLastSystemError());
    }
    else
    {
        std::unique_ptr<ChildProcessImpl> childProcess = std::make_unique<ChildProcessImpl>();
        childProcess->pid                              = pid;

        HANDLE_EINTR(close(execEventPipe.at(1)));
        LinuxFileDescriptor execEventFd(execEventPipe.at(0));
        if (stdoutPipe.has_value())
        {
            HANDLE_EINTR(close(stdoutPipe->at(1)));
            childProcess->stdoutReadFd = stdoutPipe->at(0);
        }
        if (stderrPipe.has_value())
        {
            HANDLE_EINTR(close(stderrPipe->at(1)));
            childProcess->stderrReadFd = stderrPipe->at(0);
        }
        if (stdinPipe.has_value())
        {
            HANDLE_EINTR(close(stdinPipe->at(0)));
            childProcess->stdinWriteFd = stdinPipe->at(1);
        }
        int  error  = 0;
        auto result = HANDLE_EINTR(read(execEventPipe.at(0), &error, sizeof(error)));
        if (result > 0)
        {
            return zeus::unexpected(SystemError {error});
        }
        else if (result < 0)
        {
            return zeus::unexpected(GetLastSystemError());
        }

        if (stdoutPipe.has_value())
        {
            childProcess->stdoutReadBuffer.emplace(childProcess->stdoutReadFd.Fd(), BasicStreamBuffer::BufferMode::Read);
            childProcess->stdoutReadStream.emplace(&childProcess->stdoutReadBuffer.value());
        }
        if (stderrPipe.has_value())
        {
            childProcess->stderrReadBuffer.emplace(childProcess->stderrReadFd.Fd(), BasicStreamBuffer::BufferMode::Read);
            childProcess->stderrReadStream.emplace(&childProcess->stderrReadBuffer.value());
        }
        if (stdinPipe.has_value())
        {
            childProcess->stdinWriteBuffer.emplace(childProcess->stdinWriteFd.Fd(), BasicStreamBuffer::BufferMode::Write);
            childProcess->stdinWriteStream.emplace(&childProcess->stdinWriteBuffer.value());
        }
        return childProcess;
    }
}
} // namespace

ChildProcess::ChildProcess() : _impl(std::make_unique<ChildProcessImpl>())
{
}

ChildProcess::~ChildProcess()
{
}

ChildProcess::ChildProcess(ChildProcess&& other) noexcept : _impl(std::move(other._impl))
{
}

zeus::expected<int32_t, std::error_code> ChildProcess::Wait(const std::chrono::steady_clock::duration& timeout)
{
    assert(_impl->pid > 0);
    std::chrono::microseconds           period(100);
    std::chrono::steady_clock::duration duration(0);
    while (true)
    {
        int  status = 0;
        auto result = HANDLE_EINTR(waitpid(_impl->pid, &status, WNOHANG));
        if (result < 0)
        {
            return zeus::unexpected(GetLastSystemError());
        }
        else if (result > 0)
        {
            if (WIFEXITED(status))
            {
                return WEXITSTATUS(status);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (duration > timeout)
            {
                return zeus::unexpected(SystemError {ETIMEDOUT});
            }
            Sleep(period);
            duration += period;
        }
    }
}

zeus::Process::PID ChildProcess::GetPID()
{
    return _impl->pid;
}

zeus::expected<void, std::error_code> ChildProcess::Kill()
{
    return Process::KillProcess(_impl->pid, std::chrono::milliseconds(0));
}

bool ChildProcess::HasStdout()
{
    return _impl->stdoutReadStream.has_value();
}

bool ChildProcess::HasStderr()
{
    return _impl->stderrReadStream.has_value();
}

bool ChildProcess::HasStdin()
{
    return _impl->stdinWriteStream.has_value();
}

std::istream& ChildProcess::GetStdout()
{
    return *_impl->stdoutReadStream;
}

std::istream& ChildProcess::GetStderr()
{
    return *_impl->stderrReadStream;
}

std::ostream& ChildProcess::GetStdin()
{
    return *_impl->stdinWriteStream;
}

zeus::expected<ChildProcess, std::error_code> ChildProcessExecutor::ExecuteScript(std::string_view script, const std::vector<std::string>& args)
{
    std::vector<std::string> cmdArgs;
    cmdArgs.reserve(1 + args.size());
    cmdArgs.emplace_back(std::string(script));
    for (const auto& arg : args)
    {
        cmdArgs.emplace_back(arg);
    }
    auto childProcessImpl = CreateLinuxProcess(
        "/bin/bash", cmdArgs, _impl->environmentVariables, _impl->workDir, _impl->inheritedFds, _impl->redirectStdout, _impl->redirectStderr,
        _impl->redirectStdin, _impl->uid, _impl->gid, _impl->disableStdout, _impl->disableStderr, _impl->disableStdin
    );
    if (childProcessImpl.has_value())
    {
        ChildProcess child;
        child._impl = std::move(childProcessImpl.value());
        return child;
    }
    else
    {
        return zeus::unexpected(childProcessImpl.error());
    }
}

zeus::expected<ChildProcess, std::error_code> ChildProcessExecutor::ExecuteCommand(std::string_view cmd)
{
    std::vector<std::string> cmdArgs          = {std::string("-c"), std::string(cmd)};
    auto                     childProcessImpl = CreateLinuxProcess(
        "/bin/bash", cmdArgs, _impl->environmentVariables, _impl->workDir, _impl->inheritedFds, _impl->redirectStdout, _impl->redirectStderr,
        _impl->redirectStdin, _impl->uid, _impl->gid, _impl->disableStdout, _impl->disableStderr, _impl->disableStdin
    );
    if (childProcessImpl.has_value())
    {
        ChildProcess child;
        child._impl = std::move(childProcessImpl.value());
        return child;
    }
    else
    {
        return zeus::unexpected(childProcessImpl.error());
    }
}

zeus::expected<ChildProcess, std::error_code> ChildProcessExecutor::ExecuteProcess(std::string_view process, const std::vector<std::string>& args)
{
    auto childProcessImpl = CreateLinuxProcess(
        std::string(process), args, _impl->environmentVariables, _impl->workDir, _impl->inheritedFds, _impl->redirectStdout, _impl->redirectStderr,
        _impl->redirectStdin, _impl->uid, _impl->gid, _impl->disableStdout, _impl->disableStderr, _impl->disableStdin
    );
    if (childProcessImpl.has_value())
    {
        ChildProcess child;
        child._impl = std::move(childProcessImpl.value());
        return child;
    }
    else
    {
        return zeus::unexpected(childProcessImpl.error());
    }
}
} // namespace zeus
#endif
