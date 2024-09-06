#include "zeus/foundation/hardware/monitor.h"
#ifdef __linux__
#include <cstring>
#include <filesystem>
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "impl/monitor_impl.h"

namespace fs = std::filesystem;

namespace zeus::Hardware
{
EDIDInfo ParseEDID(const std::vector<uint8_t>& edid);
void     FillMonitorImpl(MonitorImpl& monitorImpl, const EDIDInfo& EDIDInfo);

namespace
{
EDIDInfo GetEDID(const fs::path& monitorDir)
{
    auto                 data = FileContent(monitorDir / "edid", true).value_or("");
    std::vector<uint8_t> edid(data.size());
    std::memcpy(edid.data(), data.data(), data.size());
    return ParseEDID(edid);
}

bool IsConnected(const fs::path& monitorDir)
{
    auto data = Trim(FileContent(monitorDir / "status", false).value_or(""));
    if (!data.empty())
    {
        return "connected" == data;
    }
    return false;
}
} // namespace

std::vector<Monitor> Monitor::ListAll()
{
    std::vector<Monitor> result;
    std::error_code      ec;
    for (auto& item : fs::directory_iterator("/sys/class/drm", ec))
    {
        if (!item.is_directory(ec))
        {
            continue;
        }
        const auto& monitorDir = item.path();
        if (IsConnected(monitorDir))
        {
            Monitor monitor;
            monitor._impl->id         = monitorDir.filename().string();
            monitor._impl->devicePath = monitorDir.string();
            FillMonitorImpl(*monitor._impl, GetEDID(monitorDir));
            result.emplace_back(std::move(monitor));
        }
    }
    return result;
}
} // namespace zeus::Hardware
#endif
