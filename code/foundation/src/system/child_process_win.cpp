#include "zeus/foundation/system/child_process.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <map>
#include <list>
#include <memory>
#include <sstream>
#include <cassert>
#include <Windows.h>
#include <UserEnv.h>
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/system/environment_variable.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/resource/pipe_stream_buffer.h"
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/container/container_cast.hpp"
#include "impl/child_process_impl.h"

#pragma comment(lib, "Userenv.lib")

namespace zeus
{
struct ChildProcessImpl
{
    zeus::WinHandle                 processHandle;
    zeus::Process::PID              pid = 0;
    zeus::WinHandle                 stdoutReadHandle;
    zeus::WinHandle                 stderrReadHandle;
    zeus::WinHandle                 stdinWriteHandle;
    std::optional<PipeStreamBuffer> stdoutReadBuffer;
    std::optional<PipeStreamBuffer> stderrReadBuffer;
    std::optional<PipeStreamBuffer> stdinWriteBuffer;
    std::optional<std::istream>     stdoutReadStream;
    std::optional<std::istream>     stderrReadStream;
    std::optional<std::ostream>     stdinWriteStream;
};
namespace
{

std::shared_ptr<uint8_t> CreateWindowsEnvironmentBlock(
    const std::map<std::string, std::string>& environmentVariables, LPVOID tokenEnvironment = nullptr
)
{
    using namespace std::placeholders;
    if (!environmentVariables.empty())
    {
        std::vector<uint8_t> data;
        if (!tokenEnvironment)
        {
            data = EnvironmentVariable::CreateEnvironmentVariableData(MapToMap<std::string, std::string, std::wstring, std::wstring>(
                environmentVariables, std::bind(&CharsetUtils::UTF8ToUnicode, std::placeholders::_1),
                std::bind(&CharsetUtils::UTF8ToUnicode, std::placeholders::_1)
            ));
        }
        else
        {
            std::map<std::wstring, std::wstring> envs = MapToMap<std::string, std::string, std::wstring, std::wstring>(
                environmentVariables, std::bind(&CharsetUtils::UTF8ToUnicode, std::placeholders::_1),
                std::bind(&CharsetUtils::UTF8ToUnicode, std::placeholders::_1)
            );
            auto tokenEnvData = EnvironmentVariable::ParseEnvironmentVariableDataW(tokenEnvironment);
            for (const auto& [key, value] : tokenEnvData)
            {
                envs.emplace(key, value);
            }
            data = EnvironmentVariable::CreateEnvironmentVariableData(envs);
        }
        auto result = std::shared_ptr<uint8_t>(new uint8_t[data.size()], std::default_delete<uint8_t[]>());
        std::memcpy(result.get(), data.data(), data.size());
        return result;
    }

    if (tokenEnvironment && environmentVariables.empty())
    {
        return std::shared_ptr<uint8_t>(static_cast<uint8_t*>(tokenEnvironment), [](uint8_t*) {});
    }

    return nullptr;
}

zeus::expected<std::unique_ptr<ChildProcessImpl>, std::error_code> CreateWinProcess(
    std::string_view process, const std::vector<std::string>& args, const std::map<std::string, std::string>& environmentVariables,
    const std::string& workDir, bool show, const std::list<HANDLE>& inheritedHandles, HANDLE token, bool useTokenEnv, bool redirectStdout,
    bool redirectStderr, bool redirectStdin
)
{
    std::wstring               exe = CharsetUtils::UTF8ToUnicode(process);
    std::unique_ptr<wchar_t[]> cmd; // NOLINT(cppcoreguidelines-avoid-c-arrays)  没办法编译期计算数组长度，无法使用std::array
    if (!args.empty())
    {
        std::wstring cmdLine;
        cmdLine.reserve(process.size() * args.size() * 10);
        if (std::wstring::npos != exe.find(L' '))
        {
            cmdLine.append(LR"(")");
            cmdLine.append(exe);
            cmdLine.append(LR"(")");
        }
        else
        {
            cmdLine.append(exe);
        }
        for (const auto& arg : args)
        {
            cmdLine.push_back(L' ');
            if (std::string::npos != arg.find(' ') && !zeus::StartWith(arg, (R"(")")))
            {
                cmdLine.append(LR"(")");
                cmdLine.append(CharsetUtils::UTF8ToUnicode(arg));
                cmdLine.append(LR"(")");
            }
            else
            {
                cmdLine.append(CharsetUtils::UTF8ToUnicode(arg));
            }
        }
        //这里把命令行再拷贝一遍是因为CreateProcess要求传入的命令行参数不能是const的指针
        cmd =
            std::make_unique<wchar_t[]>(cmdLine.size() + 1); // NOLINT(cppcoreguidelines-avoid-c-arrays)  没办法编译期计算数组长度，无法使用std::array
        memset(cmd.get(), 0, sizeof(wchar_t) * (cmdLine.size() + 1));
        memcpy(cmd.get(), cmdLine.data(), sizeof(wchar_t) * cmdLine.size());
    }
    else
    {
        cmd = std::make_unique<wchar_t[]>(exe.size() + 1); // NOLINT(cppcoreguidelines-avoid-c-arrays)  没办法编译期计算数组长度，无法使用std::array
        memset(cmd.get(), 0, sizeof(wchar_t) * (exe.size() + 1));
        memcpy(cmd.get(), exe.data(), sizeof(wchar_t) * exe.size());
    }
    auto      allInheritedHandles = ListToVector(inheritedHandles);
    WinHandle stdoutRead;
    WinHandle stdoutWrite;
    WinHandle stderrRead;
    WinHandle stderrWrite;
    WinHandle stdinRead;
    WinHandle stdinWrite;
    if (redirectStdout)
    {
        CreatePipe(&stdoutRead, &stdoutWrite, nullptr, 0);
        if (stdoutWrite)
        {
            stdoutWrite.SetInherite(true);
            allInheritedHandles.emplace_back(stdoutWrite);
        }
    }
    if (redirectStderr)
    {
        CreatePipe(&stderrRead, &stderrWrite, nullptr, 0);
        if (stderrWrite)
        {
            stderrWrite.SetInherite(true);
            allInheritedHandles.emplace_back(stderrWrite);
        }
    }
    if (redirectStdin)
    {
        CreatePipe(&stdinRead, &stdinWrite, nullptr, 0);
        if (stdinRead)
        {
            stdinRead.SetInherite(true);
            allInheritedHandles.emplace_back(stdinRead);
        }
    }

    STARTUPINFOEXW si {};
    std::wstring   desktop(L"WinSta0\\Default");
    si.StartupInfo.lpDesktop =
        const_cast<wchar_t*>(desktop.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast) Windows定义必须用非const，没有完美的办法解决这个警告。
    si.StartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
    si.StartupInfo.wShowWindow = show ? SW_SHOW : SW_HIDE;

    if (stdoutWrite)
    {
        si.StartupInfo.hStdOutput = stdoutWrite;
        si.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    }
    if (stderrWrite)
    {
        si.StartupInfo.hStdError = stderrWrite;
        si.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    }
    if (stdinRead)
    {
        si.StartupInfo.hStdInput = stdinRead;
        si.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    }
    const bool                   inherited               = !allInheritedHandles.empty();
    LPPROC_THREAD_ATTRIBUTE_LIST procThreadAttributeList = nullptr;
    AutoRelease                  procThreadAttributeListRelease(
        [&procThreadAttributeList]()
        {
            if (procThreadAttributeList)
            {
                DeleteProcThreadAttributeList(procThreadAttributeList);
                free(procThreadAttributeList);
                procThreadAttributeList = nullptr;
            }
        }
    );
    if (inherited)
    {
        SIZE_T procThreadAttributeListSize = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &procThreadAttributeListSize);
        if (0 == procThreadAttributeListSize)
        {
            return zeus::unexpected(GetLastSystemError());
        }
        procThreadAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(procThreadAttributeListSize));
        if (!InitializeProcThreadAttributeList(procThreadAttributeList, 1, 0, &procThreadAttributeListSize))
        {
            return zeus::unexpected(GetLastSystemError());
        }
        if (!UpdateProcThreadAttribute(
                procThreadAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, allInheritedHandles.data(),
                allInheritedHandles.size() * sizeof(HANDLE), nullptr, nullptr
            ))
        {
            return zeus::unexpected(GetLastSystemError());
        }
        si.lpAttributeList = procThreadAttributeList;
    }
    if (inherited)
    {
        si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    }
    else
    {
        si.StartupInfo.cb = sizeof(STARTUPINFOW);
    }

    LPVOID                       tokenEnvBlock = nullptr;
    std::unique_ptr<AutoRelease> tokenEnvRelease;
    if (token && useTokenEnv)
    {
        if (!::CreateEnvironmentBlock(&tokenEnvBlock, token, FALSE))
        {
            return zeus::unexpected(GetLastSystemError());
        }
        tokenEnvRelease = std::make_unique<AutoRelease>(
            [tokenEnvBlock]()
            {
                if (tokenEnvBlock)
                {
                    ::DestroyEnvironmentBlock(tokenEnvBlock);
                }
            }
        );
    }
    else
    {
        tokenEnvBlock = GetEnvironmentStringsW();
    }

    auto                newEnvBlock = CreateWindowsEnvironmentBlock(environmentVariables, tokenEnvBlock);
    auto                wWorkDir    = CharsetUtils::UTF8ToUnicode(workDir);
    PROCESS_INFORMATION pi          = PROCESS_INFORMATION();
    BOOL                ret         = FALSE;
    if (token)
    {
        ret = CreateProcessAsUserW(
            token, nullptr, cmd.get(), nullptr, nullptr, inherited ? TRUE : FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | (inherited ? EXTENDED_STARTUPINFO_PRESENT : 0) | CREATE_UNICODE_ENVIRONMENT,
            newEnvBlock.get(), wWorkDir.empty() ? nullptr : wWorkDir.c_str(), &si.StartupInfo, &pi
        );
    }
    else
    {
        ret = CreateProcessW(
            nullptr, cmd.get(), nullptr, nullptr, inherited ? TRUE : FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | (inherited ? EXTENDED_STARTUPINFO_PRESENT : 0) | CREATE_UNICODE_ENVIRONMENT,
            newEnvBlock.get(), wWorkDir.empty() ? nullptr : wWorkDir.c_str(), &si.StartupInfo, &pi
        );
    }

    if (ret)
    {
        auto childProcess = std::make_unique<ChildProcessImpl>();
        if (stdoutRead)
        {
            childProcess->stdoutReadHandle = std::move(stdoutRead);
            childProcess->stdoutReadBuffer.emplace(childProcess->stdoutReadHandle, BasicStreamBuffer::BufferMode::Read);
            childProcess->stdoutReadStream.emplace(&childProcess->stdoutReadBuffer.value());
        }
        if (stderrRead)
        {
            childProcess->stderrReadHandle = std::move(stderrRead);
            childProcess->stderrReadBuffer.emplace(childProcess->stderrReadHandle, BasicStreamBuffer::BufferMode::Read);
            childProcess->stderrReadStream.emplace(&childProcess->stderrReadBuffer.value());
        }
        if (stdinWrite)
        {
            childProcess->stdinWriteHandle = std::move(stdinWrite);
            childProcess->stdinWriteBuffer.emplace(childProcess->stdinWriteHandle, BasicStreamBuffer::BufferMode::Write);
            childProcess->stdinWriteStream.emplace(&childProcess->stdinWriteBuffer.value());
        }
        childProcess->pid           = pi.dwProcessId;
        childProcess->processHandle = pi.hProcess;
        if (pi.hThread)
        {
            CloseHandle(pi.hThread);
        }
        return std::move(childProcess);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
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
    assert(!_impl->processHandle.Empty());
    const auto wait =
        WaitForSingleObject(_impl->processHandle, static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()));
    switch (wait)
    {
    case WAIT_OBJECT_0:
    {
        DWORD exitCode = 0;
        if (GetExitCodeProcess(_impl->processHandle, &exitCode))
        {
            return exitCode;
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    case WAIT_TIMEOUT:
    {
        return zeus::unexpected(SystemError {WAIT_TIMEOUT});
    }
    default:
    {
        return zeus::unexpected(GetLastSystemError());
    }
    }
}

zeus::Process::PID ChildProcess::GetPID()
{
    return _impl->pid;
}

zeus::expected<void, std::error_code> ChildProcess::Kill(uint32_t code)
{
    assert(!_impl->processHandle.Empty());
    if (TerminateProcess(_impl->processHandle, code))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
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
    std::string cmdArg;
    cmdArg.reserve(script.size() * args.size() * 10);
    cmdArg.append(R"(" )"); //执行cmd /c 时需要使用双引号扩住整个命令行" a.bat /a ",命令行参数有空格的再次用双引号扩住
                            //  " "a b.bat" "/a c" "
    if (std::string::npos != script.find(' ') && !zeus::StartWith(script, (R"(")")))
    {
        cmdArg.append(R"(")");
        cmdArg.append(script);
        cmdArg.append(R"(")");
    }
    else
    {
        cmdArg.append(script);
    }
    cmdArg.append(R"( )");
    for (const auto& arg : args)
    {
        if (std::string::npos != arg.find(' ') && !zeus::StartWith(arg, (R"(")")))
        {
            cmdArg.append(R"(")");
            cmdArg.append(arg);
            cmdArg.append(R"(")");
        }
        else
        {
            cmdArg.append(arg);
        }
        cmdArg.append(R"( )");
    }
    cmdArg.append(R"(")");
    std::vector<std::string> cmdArgs          = {std::string("/c"), cmdArg};
    auto                     childProcessImpl = CreateWinProcess(
        "cmd.exe", cmdArgs, _impl->environmentVariables, _impl->workDir, _impl->show, _impl->inheritedHandles, _impl->token, _impl->useTokenEnv,
        _impl->redirectStdout, _impl->redirectStderr, _impl->redirectStdin
    );
    if (childProcessImpl.has_value())
    {
        ChildProcess child;
        child._impl = std::move(childProcessImpl.value());
        return (std::move(child));
    }
    else
    {
        return zeus::unexpected(childProcessImpl.error());
    }
}

zeus::expected<ChildProcess, std::error_code> ChildProcessExecutor::ExecuteCommand(std::string_view cmd)
{
    std::vector<std::string> cmdArgs          = {std::string("/c"), R"(" )" + std::string(cmd) + R"( ")"};
    auto                     childProcessImpl = CreateWinProcess(
        "cmd.exe", cmdArgs, _impl->environmentVariables, _impl->workDir, _impl->show, _impl->inheritedHandles, _impl->token, _impl->useTokenEnv,
        _impl->redirectStdout, _impl->redirectStderr, _impl->redirectStdin
    );
    if (childProcessImpl.has_value())
    {
        ChildProcess child;
        child._impl = std::move(childProcessImpl.value());
        return (std::move(child));
    }
    else
    {
        return zeus::unexpected(childProcessImpl.error());
    }
}

zeus::expected<ChildProcess, std::error_code> ChildProcessExecutor::ExecuteProcess(std::string_view process, const std::vector<std::string>& args)
{
    auto childProcessImpl = CreateWinProcess(
        process, args, _impl->environmentVariables, _impl->workDir, _impl->show, _impl->inheritedHandles, _impl->token, _impl->useTokenEnv,
        _impl->redirectStdout, _impl->redirectStderr, _impl->redirectStdin
    );
    if (childProcessImpl.has_value())
    {
        ChildProcess child;
        child._impl = std::move(childProcessImpl.value());
        return (std::move(child));
    }
    else
    {
        return zeus::unexpected(childProcessImpl.error());
    }
}

} // namespace zeus
#endif
