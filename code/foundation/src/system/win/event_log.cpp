#include "zeus/foundation/system/win/event_log.h"
#ifdef _WIN32
#include <windows.h>
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/system/current_exe.h"

namespace zeus
{
bool WinEventLog::ReportEventLog(const std::string& source, uint32_t id, const std::string& message, WinEventLogType type)
{
    std::wstring sourceName;
    if (source.empty())
    {
        sourceName = CharsetUtils::UTF8ToUnicode(CurrentExe::GetAppName());
    }
    else
    {
        sourceName = CharsetUtils::UTF8ToUnicode(source);
    }
    HANDLE handle = RegisterEventSourceW(nullptr, sourceName.c_str());

    if (handle)
    {
        AutoRelease release([handle]() { DeregisterEventSource(handle); });
        auto        wmessage  = CharsetUtils::UTF8ToUnicode(message);
        LPCWSTR     input[]   = {wmessage.c_str()};
        WORD        eventType = EVENTLOG_INFORMATION_TYPE;
        switch (type)
        {
        case WinEventLogType::LOG_TYPE_INFO:
            eventType = EVENTLOG_INFORMATION_TYPE;
            break;
        case WinEventLogType::LOG_TYPE_WARN:
            eventType = EVENTLOG_WARNING_TYPE;
            break;
        case WinEventLogType::LOG_TYPE_ERROR:
            eventType = EVENTLOG_ERROR_TYPE;
            break;
        }
        if (ReportEventW(
                handle,    // Event log handle.
                eventType, // Event type.
                0,         // Event category.
                id,        // Event identifier.
                nullptr,   // No user security identifier.
                1,         // Number of substitution strings.
                0,         // No data.
                input,     // Pointer to strings.
                nullptr
            ))             // No data.
        {
            return true;
        }
    }
    return false;
}
} // namespace zeus
#endif
