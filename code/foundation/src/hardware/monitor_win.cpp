#include "zeus/foundation/hardware/monitor.h"
#ifdef _WIN32
#include <cstring>
#include <Windows.h>
#include "zeus/foundation/system/win/registry.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/byte/byte_order.h"
#include "zeus/foundation/system/win/device_setup.h"
#include "impl/monitor_impl.h"

namespace zeus::Hardware
{
EDIDInfo ParseEDID(const std::vector<uint8_t>& edid);
void     FillMonitorImpl(MonitorImpl& monitorImpl, const EDIDInfo& EDIDInfo);
namespace
{
EDIDInfo GetEDID(const std::string& deviceId)
{
    auto monitorReg =
        WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, R"(SYSTEM\CurrentControlSet\Enum\DISPLAY\)" + deviceId + R"(\Device Parameters)", true, false);
    if (!monitorReg)
    {
        return {};
    }
    auto data = monitorReg->GetBinaryValue("EDID");
    if (!data.has_value())
    {
        return {};
    }
    return ParseEDID(*data);
}
}

std::vector<Monitor> Monitor::ListAll()
{
    static const std::string kDisplayInstancePathPrefix = R"(DISPLAY)";
    std::vector<Monitor>     result;
    auto                     monitors = WinDeviceSetup::List(WinDeviceSetup::DeviceType::kMONITOR);
    for (auto& device : monitors)
    {
        auto instancePath = device.GetDeviceInstancePath();
        if (instancePath && !instancePath->empty())
        {
            auto& path  = *instancePath;
            auto  split = zeus::Split(path, R"(\)");

            if (3 == split.size() && kDisplayInstancePathPrefix == split.at(0))
            {
                auto    deviceId = split.at(1) + R"(\)" + split.at(2);
                auto    EDIDInfo = GetEDID(deviceId);
                Monitor monitor;
                FillMonitorImpl(*monitor._impl, EDIDInfo);
                monitor._impl->id         = deviceId;
                monitor._impl->devicePath = path;
                result.emplace_back(std::move(monitor));
            }
        }
    }
    return result;
}
} // namespace zeus::Hardware
#endif
