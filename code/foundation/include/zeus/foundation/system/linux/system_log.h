#pragma once
#ifdef __linux__
#include <string>
namespace zeus
{
class LinuxSystemLog
{
public:
    enum LinuxEventLogType
    {
        LOG_TYPE_INFO  = 1,
        LOG_TYPE_WARN  = 2,
        LOG_TYPE_ERROR = 3,
        LOG_TYPE_DEBUG = 4,
    };
    static void ReportSystemLog(const std::string& source, const std::string& message, LinuxEventLogType type = LinuxEventLogType::LOG_TYPE_INFO);
};
}
#endif
#include "zeus/foundation/core/zeus_compatible.h"
