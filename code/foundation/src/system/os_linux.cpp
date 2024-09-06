#include "zeus/foundation/system/os.h"
#ifdef __linux__
#include <cctype>
#include <sstream>
#include <algorithm>
#include <sys/utsname.h>
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/file/kv_file_utils.h"
#include "zeus/foundation/time/time_utils.h"

namespace zeus::OS
{

std::string OsKernelName()
{
    struct utsname uts;
    uname(&uts);
    return uts.sysname;
}

std::string OsDisplayName()
{
    return GetKVFileValue("/etc/lsb-release", "DISTRIB_ID").value_or("");
}

std::string OsProductName()
{
    auto productName = OsDisplayName();
    auto type        = OsProductType();
    if (!type.empty())
    {
        productName += " " + type;
    }
    return productName;
}

zeus::Version OsVersion()
{
    std::string osVersionString = OsVersionString();
    if (auto pos = osVersionString.find_first_of("0123456789."); pos != std::string::npos)
    {
        osVersionString = osVersionString.substr(pos);
    }
    auto version = zeus::Version::Parse(osVersionString);
    if (version)
    {
        return version.value();
    }
    else
    {
        return zeus::Version();
    }
}

std::string OsVersionString()
{
    return GetKVFileValue("/etc/lsb-release", "DISTRIB_RELEASE").value_or("");
}

zeus::Version OsKernelVersion()
{
    struct utsname uts;
    uname(&uts);
    auto result = zeus::Version::Parse(uts.release);
    if (result)
    {
        return result.value();
    }
    else
    {
        return zeus::Version();
    }
}

std::string OsKernelVersionString()
{
    struct utsname uts;
    uname(&uts);
    return uts.release;
}

std::string OsArchitecture()
{
    struct utsname uts;
    uname(&uts);
    return uts.machine;
}

bool IsOs64Bits()
{
    return true;
}

std::string OsProductType()
{
    do
    {
        auto type = GetKVFileValue("/etc/lsb-release", "DISTRIB_VERSION_TYPE");
        if (!type || type->empty())
        {
            break;
        }
        return *type;
    }
    while (false);

    do
    {
        auto desc = GetKVFileValueUnquoted("/etc/lsb-release", "DISTRIB_DESCRIPTION");
        if (!desc || desc->empty())
        {
            break;
        }
        auto split = Split(*desc, " ");
        if (3 != split.size())
        {
            break;
        }
        auto type = split[2];
        if (std::all_of(type.begin(), type.end(), [](char c) { return std::isdigit(c); }))
        {
            break;
        }
        return type;
    }
    while (false);
    return std::string();
}

std::string OsFullName()
{
    std::stringstream stream;
    stream.imbue(std::locale::classic());

    stream << OsProductName() << " ";

    stream << OsVersionString() << "-" << OsArchitecture();
    return stream.str();
}

std::chrono::system_clock::time_point OsBootTime()
{
    return std::chrono::system_clock::now() - std::chrono::duration_cast<std::chrono::system_clock::duration>(OsUpDuration());
}

std::chrono::steady_clock::duration OsUpDuration()
{
    timespec time = {};
    if (0 == clock_gettime(CLOCK_BOOTTIME, &time))
    {
        return TimeSpecToDuration(time);
    }
    return std::chrono::steady_clock::duration::zero();
}

bool IsUnix()
{
    return true;
}

bool IsWindows()
{
    return false;
}
} // namespace zeus::OS
#endif