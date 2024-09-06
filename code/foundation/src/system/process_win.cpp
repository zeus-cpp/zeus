#include "zeus/foundation/system/process.h"
#ifdef _WIN32
#include <functional>
#include <Windows.h>
#include <shellapi.h>
#include <winternl.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/time/time_utils.h"
#include "impl/process_impl.h"

#pragma comment(lib, "ntdll.lib")

namespace fs = std::filesystem;

namespace zeus
{

namespace
{

typedef NTSTATUS(WINAPI *PFNNtQuerySystemInformation)(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,  // 定义服务类型号
    PVOID                    SystemInformation,       // 用户存储信息的缓冲区
    ULONG                    SystemInformationLength, // 缓冲区大小
    PULONG                   ReturnLength
);                                                    // 返回信息长度

typedef NTSTATUS(WINAPI *PFNNtQueryProcessInformation)(
    HANDLE           ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,  // 定义服务类型号
    PVOID            ProcessInformation,       // 用户存储信息的缓冲区
    ULONG            ProcessInformationLength, // 缓冲区大小
    PULONG           ReturnLength
);                                             // 返回信息长度

typedef LONG KPRIORITY;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
#if _WIN32_WINNT >= 0x0600
    LARGE_INTEGER WorkingSetPrivateSize; // since VISTA
#endif
#if _WIN32_WINNT >= 0x0601
    ULONG     HardFaultCount;               // since WIN7
    ULONG     NumberOfThreadsHighWatermark; // since WIN7
    ULONGLONG CycleTime;                    // since WIN7
#endif
    LARGE_INTEGER  CreateTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY      BasePriority;
    HANDLE         UniqueProcessId;
    HANDLE         InheritedFromUniqueProcessId;
    ULONG          HandleCount;
    ULONG          SessionId;
#if _WIN32_WINNT >= 0x0600
    ULONG_PTR UniqueProcessKey; // since VISTA (requires SystemExtendedProcessInformation)
#endif
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG         PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
    SIZE_T        PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    //SYSTEM_THREAD_INFORMATION Threads[1];

} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _PROCESS_BASIC_INFORMATION
{
    NTSTATUS  ExitStatus;
    PPEB      PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;

void EnumProcess(const std::function<bool(SYSTEM_PROCESS_INFORMATION *info)> &handler)
{
    static auto *pfnNtQuerySysInfo =
        reinterpret_cast<PFNNtQuerySystemInformation>(GetProcAddress(LoadLibraryW(L"ntdll.dll"), "NtQuerySystemInformation"));

    if (!pfnNtQuerySysInfo)
    {
        return;
    }

    DWORD                      length = 0;
    NTSTATUS                   status = 0;
    static std::atomic<size_t> bufferSize(1024 * 200);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(bufferSize);
    status                            = pfnNtQuerySysInfo(SYSTEM_INFORMATION_CLASS::SystemProcessInformation, buffer.get(), bufferSize, &length);
    while (0 != status)
    {
        bufferSize = length;
        buffer     = std::make_unique<uint8_t[]>(bufferSize);
        status     = pfnNtQuerySysInfo(SYSTEM_INFORMATION_CLASS::SystemProcessInformation, buffer.get(), bufferSize, &length);
    }
    bool  find = true;
    auto *info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION *>(buffer.get());
    while (find)
    {
        if (!handler(info))
        {
            return;
        }
        find = info->NextEntryOffset != 0;
        info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION *>(reinterpret_cast<char *>(info) + info->NextEntryOffset);
    }
}

zeus::expected<std::string, std::error_code> GetFullExeName(Process::PID pid)
{
    const size_t    MAX_PATH_LENGTH = MAX_PATH * 2;
    zeus::WinHandle handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
    if (!handle.Empty())
    {
        wchar_t imagePath[MAX_PATH_LENGTH] = {0};
        DWORD   size                       = MAX_PATH_LENGTH;
        if (QueryFullProcessImageNameW(handle, 0, imagePath, &size))
        {
            return CharsetUtils::UnicodeToUTF8(imagePath);
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    return zeus::unexpected(GetLastSystemError());
}

void FillProcessImpl(ProcessImpl &impl, SYSTEM_PROCESS_INFORMATION *info)
{
    impl.pid = reinterpret_cast<Process::PID>(info->UniqueProcessId);
    if (impl.name.empty())
    {
        impl.name = CharsetUtils::UnicodeToUTF8(std::wstring(info->ImageName.Buffer, info->ImageName.Length / sizeof(wchar_t)));
    }
    impl.ppid           = reinterpret_cast<Process::PID>(info->InheritedFromUniqueProcessId);
    impl.createTime     = FiletimeToSystemTime(info->CreateTime);
    impl.createTimeTick = info->CreateTime.QuadPart;
    if (impl.exePathString.empty())
    {
        impl.exePathString = GetFullExeName(impl.pid).value_or("");
    }
    impl.exePath   = fs::u8path(impl.exePathString);
    impl.sessionId = info->SessionId;
}
} // namespace

Process::PID Process::GetCurrentId()
{
    return GetCurrentProcessId();
}
Process Process::GetCurrentProcess()
{
    return *FindProcessById(GetCurrentProcessId());
}

std::set<Process> Process::FindProcessByPath(std::string_view path, bool ignoreCase)
{
    std::set<Process> processes;
    EnumProcess(
        [&processes, expectPath = path, ignoreCase](SYSTEM_PROCESS_INFORMATION *info)
        {
            auto path = GetFullExeName(reinterpret_cast<PID>(info->UniqueProcessId)).value_or("");
            if (ignoreCase ? IEqual(expectPath, path) : expectPath == path)
            {
                Process process;
                process._impl->exePathString = path;
                FillProcessImpl(*process._impl, info);
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
        [&processes, expectName = name, ignoreCase](SYSTEM_PROCESS_INFORMATION *info)
        {
            auto name = CharsetUtils::UnicodeToUTF8(std::wstring(info->ImageName.Buffer, info->ImageName.Length / sizeof(wchar_t)));
            if (ignoreCase ? IEqual(expectName, name) : expectName == name)
            {
                Process process;
                process._impl->name = name;
                FillProcessImpl(*process._impl, info);
                processes.emplace(std::move(process));
            }
            return true;
        }
    );
    return processes;
}
std::optional<Process> Process::FindProcessById(const PID &pid)
{
    std::optional<Process> processes;
    EnumProcess(
        [&processes, pid](SYSTEM_PROCESS_INFORMATION *info)
        {
            if (reinterpret_cast<PID>(info->UniqueProcessId) == pid)
            {
                Process process;
                FillProcessImpl(*process._impl, info);
                processes.emplace(std::move(process));
                return false;
            }
            else
            {
                return true;
            }
        }
    );
    return processes;
}
std::set<Process::PID> Process::ListId()
{
    std::set<Process::PID> processes;
    EnumProcess(
        [&processes](SYSTEM_PROCESS_INFORMATION *info)
        {
            processes.emplace(reinterpret_cast<PID>(info->UniqueProcessId));
            return true;
        }
    );
    return processes;
}
std::set<Process> Process::ListAll()
{
    std::set<Process> processes;
    EnumProcess(
        [&processes](SYSTEM_PROCESS_INFORMATION *info)
        {
            Process process;
            FillProcessImpl(*process._impl, info);
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
        [&processes](SYSTEM_PROCESS_INFORMATION *info)
        {
            Process process;
            FillProcessImpl(*process._impl, info);
            processes.emplace(reinterpret_cast<Process::PID>(info->UniqueProcessId), std::move(process));
            return true;
        }
    );
    return processes;
}

zeus::expected<void, std::error_code> Process::KillProcess(const PID &pid, const std::chrono::steady_clock::duration &timeout)
{
    WinHandle process = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (process)
    {
        if (::TerminateProcess(process, 0) != 0)
        {
            const auto wait = WaitForSingleObject(process, std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
            switch (wait)
            {
            case WAIT_OBJECT_0:
                return {};
                break;
            case WAIT_TIMEOUT:
                return zeus::unexpected(SystemError {WAIT_TIMEOUT});
                break;
            default:
                return zeus::unexpected(GetLastSystemError());
            }
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<std::filesystem::path, std::error_code> Process::GetProcessExePath(const PID &pid)
{
    auto path = GetFullExeName(pid);
    if (path.has_value())
    {
        return fs::u8path(path.value());
    }
    return zeus::unexpected(path.error());
}
zeus::expected<std::string, std::error_code> Process::GetProcessExePathString(const PID &pid)
{
    return GetFullExeName(pid);
}

zeus::expected<std::vector<std::string>, std::error_code> Process::GetProcessCmdlineArguments(const PID &pid)
{
    static auto *pfnNtQueryProcessInfo =
        reinterpret_cast<PFNNtQueryProcessInformation>(GetProcAddress(LoadLibraryW(L"ntdll.dll"), "NtQueryInformationProcess"));

    if (!pfnNtQueryProcessInfo)
    {
        return zeus::unexpected(SystemError {ERROR_PROC_NOT_FOUND});
    }
    PROCESS_BASIC_INFORMATION info   = {};
    WinHandle                 handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    const NTSTATUS result = pfnNtQueryProcessInfo(handle, ProcessBasicInformation, &info, sizeof(info), nullptr);
    if (result != 0)
    {
        return zeus::unexpected(SystemError {RtlNtStatusToDosError(result)});
    }
    PEB                         peb   = {};
    RTL_USER_PROCESS_PARAMETERS param = {};
    if (!ReadProcessMemory(handle, info.PebBaseAddress, &peb, sizeof(PEB), nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (!ReadProcessMemory(handle, peb.ProcessParameters, &param, sizeof(param), nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (0 == param.CommandLine.Length)
    {
        return {};
    }
    auto buffer = std::make_unique<wchar_t[]>(param.CommandLine.Length / sizeof(wchar_t));
    if (!ReadProcessMemory(handle, param.CommandLine.Buffer, buffer.get(), param.CommandLine.Length, nullptr))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    auto    cmdline = std::wstring(buffer.get(), param.CommandLine.Length / sizeof(wchar_t));
    int     count   = 0;
    LPWSTR *arglist = CommandLineToArgvW(cmdline.c_str(), &count);
    if (arglist == nullptr)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    else
    {
        std::vector<std::string> args;
        for (int i = 0; i < count; ++i)
        {
            args.emplace_back(CharsetUtils::UnicodeToUTF8(arglist[i]));
        }
        LocalFree(arglist);
        return std::move(args);
    }
}
} // namespace zeus
#endif
