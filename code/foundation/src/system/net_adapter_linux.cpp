#include "zeus/foundation/system/net_adapter.h"
#ifdef __linux__
#define HAVE_PCAP
#include <filesystem>
#include <cstring>
#include <set>
#include <functional>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include "zeus/foundation/core/posix/eintr_wrapper.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/byte/byte_utils.h"
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "impl/net_adapter_impl.h"

namespace fs = std::filesystem;

namespace zeus
{
namespace
{

LinuxFileDescriptor NetSocket()
{
    return LinuxFileDescriptor(socket(AF_INET, SOCK_DGRAM, 0));
}

void EnumNetAdapter(const std::function<bool(const fs::path& adapterDir, const std::string& name)>& handler)
{
    std::error_code ec;
    for (const auto& item : fs::directory_iterator("/sys/class/net", ec))
    {
        if (!item.is_directory(ec))
        {
            continue;
        }
        if (!handler(item.path(), item.path().filename().string()))
        {
            break;
        }
    }
}

bool IsVirtualAdapter(const fs::path& adapterDir)
{
    std::error_code ec;
    return !fs::exists(adapterDir / "device", ec);
}

NetAdapter::NetAdapterType GetAdapterType(const std::string& name)
{
    if ("lo" == name)
    {
        return NetAdapter::NetAdapterType::Loopback;
    }
    auto netSocket = NetSocket();
    if (!netSocket)
    {
        return NetAdapter::NetAdapterType::Other;
    }
    struct ifreq ifr = {};
    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    if (0 == HANDLE_EINTR(ioctl(netSocket.Fd(), SIOCGIFHWADDR, &ifr)))
    {
        switch (ifr.ifr_hwaddr.sa_family)
        {
        case ARPHRD_ETHER:
        {
            struct iwreq wrq = {};
            std::strncpy(wrq.ifr_name, name.c_str(), IFNAMSIZ - 1);
            if (0 == HANDLE_EINTR(ioctl(netSocket.Fd(), SIOCGIWNAME, &wrq)))
            {
                return NetAdapter::NetAdapterType::Wireless;
            }
            return NetAdapter::NetAdapterType::Ethernet;
        }
        case ARPHRD_LOOPBACK:
            return NetAdapter::NetAdapterType::Loopback;
        default:
            return NetAdapter::NetAdapterType::Other;
        }
    }
    return NetAdapter::NetAdapterType::Other;
}

void ParseIPAddress(const std::string& name, const std::set<sa_family_t>& family, std::vector<IPV4Address>& v4List, std::vector<IPV6Address>& v6List)
{
    ifaddrs*    ifaddr = nullptr;
    AutoRelease release(
        [&ifaddr]()
        {
            if (ifaddr)
            {
                freeifaddrs(ifaddr);
            }
        }
    );
    int ret = getifaddrs(&ifaddr);
    if (0 != ret || !ifaddr)
    {
        return;
    }
    for (const auto* address = ifaddr; address; address = address->ifa_next)
    {
        if (0 != strcmp(name.c_str(), address->ifa_name))
        {
            continue;
        }
        if (!family.count(address->ifa_addr->sa_family))
        {
            continue;
        }
        if (address->ifa_addr->sa_family == AF_INET)
        {
            const auto* addr = reinterpret_cast<sockaddr_in*>(address->ifa_addr);
            const auto* mask = reinterpret_cast<sockaddr_in*>(address->ifa_netmask);
            v4List.emplace_back(IPV4Address::Parse(&(addr->sin_addr), &(mask->sin_addr)));
        }
        else if (address->ifa_addr->sa_family == AF_INET6)
        {
            const auto* addr = reinterpret_cast<sockaddr_in6*>(address->ifa_addr);
            const auto* mask = reinterpret_cast<sockaddr_in6*>(address->ifa_netmask);
            v6List.emplace_back(IPV6Address::Parse(&(addr->sin6_addr), &(mask->sin6_addr)));
        }
    }
}

int GetAdapterFlags(const std::string& name)
{
    auto netSocket = NetSocket();
    if (!netSocket)
    {
        return 0;
    }
    struct ifreq ifr = {};
    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    if (0 == HANDLE_EINTR(ioctl(netSocket.Fd(), SIOCGIFFLAGS, &ifr)))
    {
        return ifr.ifr_flags;
    }
    return 0;
}

bool IsConnectedAdapter(const std::string& name)
{
    auto flags = GetAdapterFlags(name);
    return (flags & IFF_RUNNING) && (flags & IFF_UP);
}

uint64_t GetWirelessSpeed(const std::string& name)
{
    auto netSocket = NetSocket();
    if (!netSocket)
    {
        return 0;
    }
    struct iwreq wrq = {};
    std::strncpy(wrq.ifr_name, name.c_str(), IFNAMSIZ - 1);
    if (0 != HANDLE_EINTR(ioctl(netSocket.Fd(), SIOCGIWRATE, &wrq)))
    {
        return 0;
    }
    return wrq.u.bitrate.value;
}

uint64_t GetAdapterSpeed(const fs::path& adapterDir, NetAdapter::NetAdapterType type)
{
    if (NetAdapter::NetAdapterType::Wireless == type)
    {
        return GetWirelessSpeed(adapterDir.filename().string());
    }
    auto speedString = Trim(FileContent(adapterDir / "speed").value_or(""));
    if (!speedString.empty() && !StartWith(speedString, "-"))
    {
        try
        {
            return std::stoll(speedString) * 1000 * 1000;
        }
        catch (...)
        {
        }
    }
    return 0;
}

std::vector<uint8_t> GetAdapterMACByNetdevice(const std::string& name)
{
    std::vector<uint8_t> mac;
    auto                 netSocket = NetSocket();
    if (netSocket)
    {
        struct ifreq ifr = {};
        std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
        if (0 == HANDLE_EINTR(ioctl(netSocket.Fd(), SIOCGIFHWADDR, &ifr)))
        {
            mac.resize(6);
            std::memcpy(mac.data(), ifr.ifr_hwaddr.sa_data, mac.size());
        }
    }
    return mac;
}

std::vector<uint8_t> ParseAdapterMACBySysFs(const fs::path& adapterDir)
{
    std::vector<uint8_t> mac;
    auto                 macString = Trim(FileContent(adapterDir / "address").value_or(""));
    macString                      = zeus::Replace(macString, ":", "");
    auto macHex                    = HexStringToBytes(macString);
    if (macHex.has_value() && 6 == macHex->size())
    {
        return std::move(macHex.value());
    }

    return mac;
}

void FillNetadapterImpl(
    NetAdapterImpl& impl, const fs::path& adapterDir, const std::string& name, NetAdapter::NetAdapterType adapterType,
    const std::set<sa_family_t>& family
)
{
    impl.id              = name;
    impl.displayName     = name;
    impl.deviceName      = name;
    impl.physicalAddress = GetAdapterMACByNetdevice(name);
    if (impl.physicalAddress.empty())
    {
        impl.physicalAddress = ParseAdapterMACBySysFs(adapterDir);
    }
    impl.type = adapterType;
    ParseIPAddress(name, family, impl.v4AddressList, impl.v6AddressList);
    // gateway暂未实现
    impl.speed       = GetAdapterSpeed(adapterDir, adapterType);
    impl.isVirtual   = IsVirtualAdapter(adapterDir);
    impl.isConnected = IsConnectedAdapter(name);
    impl.index       = if_nametoindex(name.c_str());
}

} // namespace

std::vector<NetAdapter> NetAdapter::List(NetAdapterType type, NetVersion version)
{
    std::vector<NetAdapter> adapters;
    EnumNetAdapter(
        [&adapters, type, version](const fs::path& adapterDir, const std::string& name)
        {
            auto adapterType = GetAdapterType(name);
            if (type != adapterType || NetAdapterType::Other == adapterType)
            {
                return true;
            }
            NetAdapter            adapter;
            std::set<sa_family_t> family;
            switch (version)
            {
            case NetVersion::V4:
                family.emplace(AF_INET);
                break;
            case NetVersion::V6:
                family.emplace(AF_INET6);
                break;
            default:
                family.emplace(AF_INET);
                family.emplace(AF_INET6);
            }
            FillNetadapterImpl(*adapter._impl, adapterDir, name, adapterType, family);
            adapters.emplace_back(std::move(adapter));
            return true;
        }
    );
    return adapters;
}

std::vector<NetAdapter> NetAdapter::ListAll()
{
    std::vector<NetAdapter> adapters;
    EnumNetAdapter(
        [&adapters](const fs::path& adapterDir, const std::string& name)
        {
            auto adapterType = GetAdapterType(name);
            if (NetAdapterType::Other == adapterType)
            {
                return true;
            }
            NetAdapter            adapter;
            std::set<sa_family_t> family = {AF_INET, AF_INET6};
            FillNetadapterImpl(*adapter._impl, adapterDir, name, adapterType, family);
            adapters.emplace_back(std::move(adapter));
            return true;
        }
    );
    return adapters;
}

std::optional<NetAdapter> NetAdapter::FindNetAdapterById(const std::string& id)
{
    std::optional<NetAdapter> adapter;
    EnumNetAdapter(
        [&adapter, &id](const fs::path& adapterDir, const std::string& name)
        {
            auto adapterType = GetAdapterType(name);
            if (NetAdapterType::Other == adapterType)
            {
                return true;
            }
            if (name == id)
            {
                adapter                      = NetAdapter();
                std::set<sa_family_t> family = {AF_INET, AF_INET6};
                FillNetadapterImpl(*adapter.value()._impl, adapterDir, name, adapterType, family);
                return false;
            }
            return true;
        }
    );
    return adapter;
}

} // namespace zeus
#endif
