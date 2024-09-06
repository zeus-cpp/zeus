#include "zeus/foundation/system/linux/system_log.h"
#ifdef __linux__
#include <syslog.h>
#include "zeus/foundation/system/current_exe.h"

namespace zeus
{
void LinuxSystemLog::ReportSystemLog(const std::string& source, const std::string& message, LinuxEventLogType type)
{
    std::string sourceName;
    if (source.empty())
    {
        sourceName = CurrentExe::GetAppName();
    }
    else
    {
        sourceName = source;
    }
    int priority = LOG_INFO;
    switch (type)
    {
    case LinuxEventLogType::LOG_TYPE_INFO:
        priority = LOG_INFO;
        break;
    case LinuxEventLogType::LOG_TYPE_WARN:
        priority = LOG_WARNING;
        break;
    case LinuxEventLogType::LOG_TYPE_ERROR:
        priority = LOG_ERR;
        break;
    case LinuxEventLogType::LOG_TYPE_DEBUG:
        priority = LOG_DEBUG;
        break;
    default:
        priority = LOG_INFO;
        break;
    }
    openlog(sourceName.c_str(), LOG_NOWAIT | LOG_PID, LOG_USER);
    syslog(priority, "%s", message.c_str());
    closelog();
}
} // namespace zeus
#endif
