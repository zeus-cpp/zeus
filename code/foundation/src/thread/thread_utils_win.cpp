#include "zeus/foundation/thread/thread_utils.h"
#ifdef _WIN32
#include <Windows.h>
#include "zeus/foundation/system/shared_library.h"
#include "zeus/foundation/string/charset_utils.h"

namespace zeus
{

static bool NewWindowsSetThreadName(const std::string& threadName)
{
    typedef HRESULT(WINAPI * SetThreadDescriptionFunction)(HANDLE, PCWSTR);
    static auto setThreadDescriptionFunction =
        zeus::SharedLibrary::Load(std::string("KernelBase.dll"))
            .transform([](const zeus::SharedLibrary& library) { return library.GetTypeSymbol<SetThreadDescriptionFunction>("SetThreadDescription"); })
            .value_or(nullptr);
    if (setThreadDescriptionFunction)
    {
        auto name = zeus::CharsetUtils::UTF8ToUnicode(threadName);
        setThreadDescriptionFunction(GetCurrentThread(), name.c_str());
        return true;
    }
    return false;
}

void SetThreadName(const std::string& threadName)
{
    if (NewWindowsSetThreadName(threadName))
    {
        return;
    }
#pragma pack(push, 8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD  dwType;     // Must be 0x1000.
        LPCSTR szName;     // Pointer to name (in user addr space).
        DWORD  dwThreadID; // Thread ID (-1=caller thread).
        DWORD  dwFlags;    // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)
    THREADNAME_INFO info;
    info.dwType                 = 0x1000;
    info.szName                 = threadName.c_str();
    info.dwThreadID             = -1;
    info.dwFlags                = 0;
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*) &info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

uint64_t GetThreadId()
{
    return GetCurrentThreadId();
}
} // namespace zeus

#endif