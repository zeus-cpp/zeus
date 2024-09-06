#include "zeus/foundation/debugger/debugger.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux__
#include <filesystem>
#include "zeus/foundation/file/kv_file_utils.h"
namespace fs = std::filesystem;
#endif
namespace zeus
{
#ifdef _WIN32
bool IsDebuggerAttached()
{
    return IsDebuggerPresent();
}
#endif
#ifdef __linux__
bool IsDebuggerAttached()
{
    auto path = fs::path("/proc/self/status");
    return 0 != std::stoul(GetKVFileValue(path, "TracerPid", ":\t").value_or("0"));
}
#endif
} // namespace zeus
