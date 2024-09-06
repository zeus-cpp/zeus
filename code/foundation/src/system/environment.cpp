#include "zeus/foundation/system/environment.h"
#include "zeus/foundation/system/net_adapter.h"
namespace zeus::Environment
{
std::filesystem::path GetHome()
{
    return std::filesystem::u8path(GetHomeString());
}
std::string GetMacId()
{
    static const std::string DEFAULT_EMPTY_MAC = "00:00:00:00:00:00";
    std::string              macId             = DEFAULT_EMPTY_MAC;
    auto                     adapterSet        = NetAdapter::ListAll();
    for (auto& adapter : adapterSet)
    {
        if (NetAdapter::NetAdapterType::Ethernet == adapter.GetType() && !adapter.IsVirtual())
        {
            auto mac = adapter.GetMac(":");
            if (!mac.empty())
            {
                macId.assign(mac);
                break;
            }
        }
    }
    if (DEFAULT_EMPTY_MAC == macId)
    {
        for (auto& adapter : adapterSet)
        {
            if (!adapter.IsVirtual() && NetAdapter::NetAdapterType::Loopback != adapter.GetType())
            {
                auto mac = adapter.GetMac(":");
                if (!mac.empty())
                {
                    macId.assign(mac);
                    break;
                }
            }
        }
    }
    return macId;
}

} // namespace zeus::Environment
