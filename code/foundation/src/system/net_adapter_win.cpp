#include "zeus/foundation/system/net_adapter.h"
#ifdef _WIN32
#include <set>
#include <map>
#include <cstring>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <NetCon.h>
#include <SetupAPI.h>
#include <Cfgmgr32.h>
#include <devguid.h>
#include <netlistmgr.h>
#include <guiddef.h>
#include <atlbase.h>
#include "zeus/foundation/container/case_map.hpp"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/resource/win/com_init.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/system/win/registry.h"
#include "zeus/foundation/system/shared_library.h"
#include "zeus/foundation/system/os.h"
#include "zeus/foundation/crypt/uuid.h"
#include "impl/net_adapter_impl.h"

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "setupapi.lib")

namespace zeus
{
namespace
{

constexpr size_t  kMACLength      = 6;
const std::string kNetRegistry    = R"(SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002bE10318})";
const bool        kIsWin11        = OS::IsWin11OrGreater();
constexpr DWORD   UN_NCF_PHYSICAL = 0x4;
constexpr DWORD   UN_NCF_HAS_UI   = 0x80;

NetAdapter::NetAdapterType CastAdapterType(IFTYPE type)
{
    switch (type)
    {
    case IF_TYPE_ETHERNET_CSMACD:
        return NetAdapter::NetAdapterType::Ethernet;
    case IF_TYPE_IEEE80211:
        return NetAdapter::NetAdapterType::Wireless;
    case IF_TYPE_SOFTWARE_LOOPBACK:
        return NetAdapter::NetAdapterType::Loopback;
    default:
        return NetAdapter::NetAdapterType::Other;
    }
}

void ParseIPAddress(const IP_ADAPTER_UNICAST_ADDRESS_LH* address, std::vector<IPV4Address>& v4List, std::vector<IPV6Address>& v6List)
{
    while (address)
    {
        if (address->Address.lpSockaddr->sa_family == AF_INET)
        {
            const auto* addr = reinterpret_cast<sockaddr_in*>(address->Address.lpSockaddr);
            v4List.emplace_back(IPV4Address::Parse(&addr->sin_addr, address->OnLinkPrefixLength));
        }
        else if (address->Address.lpSockaddr->sa_family == AF_INET6)
        {
            const auto* addr = reinterpret_cast<sockaddr_in6*>(address->Address.lpSockaddr);
            v6List.emplace_back(IPV6Address::Parse(&addr->sin6_addr, address->OnLinkPrefixLength));
        }
        else
        {
            assert(false);
        }
        address = address->Next;
    }
}

void ParseGateway(const IP_ADAPTER_GATEWAY_ADDRESS_LH* gateway, std::vector<std::string>& v4List, std::vector<std::string>& v6List)
{
    while (gateway)
    {
        if (gateway->Address.lpSockaddr->sa_family == AF_INET)
        {
            const auto* addr = reinterpret_cast<sockaddr_in*>(gateway->Address.lpSockaddr);
            v4List.emplace_back(IPV4Address::ToString(&addr->sin_addr));
        }
        else if (gateway->Address.lpSockaddr->sa_family == AF_INET6)
        {
            const auto* addr = reinterpret_cast<sockaddr_in6*>(gateway->Address.lpSockaddr);
            v6List.emplace_back(IPV6Address::ToString(&addr->sin6_addr));
        }
        else
        {
            assert(false);
        }
        gateway = gateway->Next;
    }
}

std::vector<uint8_t> ParseAdapterMAC(BYTE* data, size_t length)
{
    assert(kMACLength == length);
    std::vector<uint8_t> mac;
    mac.resize(length);
    std::memcpy(mac.data(), data, length);
    return mac;
}

bool IsVirtualAdapter(const std::string& id)
{
    auto registry = WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kNetRegistry, true, false);
    if (!registry)
    {
        return false;
    }
    auto subkeys = registry->GetSubKeys();
    if (!subkeys)
    {
        return false;
    }
    for (const auto& key : *subkeys)
    {
        auto control = WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kNetRegistry + R"(\)" + key, true, false);
        if (control && control->ExistsValue("Characteristics").value_or(false) && id == control->GetStringValue("NetCfgInstanceId").value_or(""))
        {
            const DWORD characteristics = control->GetDwordValue("Characteristics").value_or(0);
            auto        isVirtual       = !((characteristics & UN_NCF_PHYSICAL) && (characteristics & UN_NCF_HAS_UI));
            if (isVirtual && (characteristics & UN_NCF_PHYSICAL) && kIsWin11)
            {
                // win11上发现部分物理网卡的Characteristics值仅为UN_NCF_PHYSICAL，需要通过判断是否是PCI或者USB
                // 保持非win11的判断逻辑不变，避免造成额外影响
                auto instanceId = control->GetStringValue("DeviceInstanceID");
                if (instanceId && (zeus::StartWith(*instanceId, "PCI") || zeus::StartWith(*instanceId, "USB")))
                {
                    isVirtual = false;
                }
            }
            return isVirtual;
        }
    }

    return false;
}

std::set<std::string> ScanVirtualAdapter()
{
    std::set<std::string> virtualAdapters;
    do
    {
        auto registry = WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kNetRegistry, true, false);
        if (!registry)
        {
            break;
        }
        auto subkeys = registry->GetSubKeys();
        if (!subkeys)
        {
            break;
        }

        for (const auto& key : *subkeys)
        {
            auto control = WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, kNetRegistry + R"(\)" + key, true, false);
            if (control && control->ExistsValue("NetCfgInstanceId").value_or(false) && control->ExistsValue("Characteristics").value_or(false))
            {
                const DWORD characteristics = control->GetDwordValue("Characteristics").value_or(0);
                const auto  isVirtual       = !((characteristics & UN_NCF_PHYSICAL) && (characteristics & UN_NCF_HAS_UI));
                if (isVirtual && (characteristics & UN_NCF_PHYSICAL) && kIsWin11)
                {
                    // win11上发现部分物理网卡的Characteristics值仅为UN_NCF_PHYSICAL，需要通过判断是否是PCI或者USB
                    // 保持非win11的判断逻辑不变，避免造成额外影响

                    auto instanceId = control->GetStringValue("DeviceInstanceID");
                    if (instanceId && (zeus::StartWith(*instanceId, "PCI") || zeus::StartWith(*instanceId, "USB")))
                    {
                        continue;
                    }
                }
                if (isVirtual)
                {
                    virtualAdapters.emplace(control->GetStringValue("NetCfgInstanceId").value_or(""));
                }
            }
        }
    }
    while (false);
    return virtualAdapters;
}

std::map<std::wstring, bool, WStringICaseKeyCompare> GetDeviceMap()
{
    std::map<std::wstring, bool, WStringICaseKeyCompare> enableCache;
    //使用setupApi获取启用状态
    auto*                                                devClass = SetupDiGetClassDevsW(&GUID_DEVCLASS_NET, nullptr, nullptr, DIGCF_PRESENT);
    AutoRelease                                          devRelease([&devClass]() { SetupDiDestroyDeviceInfoList(devClass); });
    SP_DEVINFO_DATA                                      devInfo;
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    DWORD index    = 0;
    while (SetupDiEnumDeviceInfo(devClass, index, &devInfo))
    {
        DWORD                      requiredSize = 0;
        std::unique_ptr<wchar_t[]> name;
        {
            SetupDiGetDeviceRegistryPropertyW(devClass, &devInfo, SPDRP_FRIENDLYNAME, nullptr, nullptr, 0, &requiredSize);
            const auto error = GetLastError();
            if (error == ERROR_INSUFFICIENT_BUFFER)
            {
                name = std::make_unique<wchar_t[]>(requiredSize / 2);
                SetupDiGetDeviceRegistryPropertyW(
                    devClass, &devInfo, SPDRP_FRIENDLYNAME, nullptr, reinterpret_cast<PBYTE>(name.get()), requiredSize, &requiredSize
                );
            }
        }
        if (!name)
        {
            SetupDiGetDeviceRegistryPropertyW(devClass, &devInfo, SPDRP_DEVICEDESC, nullptr, nullptr, 0, &requiredSize);
            const auto error = GetLastError();
            if (error == ERROR_INSUFFICIENT_BUFFER)
            {
                name = std::make_unique<wchar_t[]>(requiredSize / 2);
                SetupDiGetDeviceRegistryPropertyW(
                    devClass, &devInfo, SPDRP_DEVICEDESC, nullptr, reinterpret_cast<PBYTE>(name.get()), requiredSize, &requiredSize
                );
            }
        }

        if (name)
        {
            DWORD      devStatus = 0;
            DWORD      problem   = 0;
            const auto ret       = CM_Get_DevNode_Status(&devStatus, &problem, devInfo.DevInst, 0);
            if (CR_SUCCESS == ret && (0 == (devStatus & DN_NO_SHOW_IN_DM)))
            {
                bool disable            = (devStatus & DN_HAS_PROBLEM) && (CM_PROB_HARDWARE_DISABLED == problem || CM_PROB_DISABLED == problem);
                enableCache[name.get()] = !disable;
            }
        }

        index++;
    }
    return enableCache;
}

std::unique_ptr<uint8_t[]> GetAdaptersAddressesData(ULONG family)
{
    unsigned long              size = 0;
    std::unique_ptr<uint8_t[]> data;
    unsigned long              ret = 0;
    while (ERROR_BUFFER_OVERFLOW == (ret = GetAdaptersAddresses(
                                         family, GAA_FLAG_INCLUDE_ALL_INTERFACES | GAA_FLAG_INCLUDE_GATEWAYS, nullptr,
                                         reinterpret_cast<IP_ADAPTER_ADDRESSES*>(data.get()), &size
                                     )))
    {
        data = std::make_unique<uint8_t[]>(size);
    }
    if (ret == ERROR_SUCCESS)
    {
        return data;
    }
    return nullptr;
}

void FillAdapterInfo(
    NetAdapterImpl& impl, IP_ADAPTER_ADDRESSES* ipAdapter, const std::set<std::string>& virtualAdapters,
    const std::map<std::wstring, bool, WStringICaseKeyCompare>& enableCache
)
{
    impl.id              = ipAdapter->AdapterName;
    impl.displayName     = CharsetUtils::UnicodeToUTF8(ipAdapter->FriendlyName);
    impl.deviceName      = CharsetUtils::UnicodeToUTF8(ipAdapter->Description);
    impl.physicalAddress = ParseAdapterMAC(ipAdapter->PhysicalAddress, kMACLength);
    impl.type            = CastAdapterType(ipAdapter->IfType);
    ParseIPAddress(ipAdapter->FirstUnicastAddress, impl.v4AddressList, impl.v6AddressList);
    ParseGateway(ipAdapter->FirstGatewayAddress, impl.v4GatewayList, impl.v6GatewayList);
    impl.speed       = ipAdapter->TransmitLinkSpeed;
    impl.isVirtual   = virtualAdapters.find(impl.id) != virtualAdapters.end();
    impl.isConnected = ipAdapter->OperStatus == IfOperStatusUp;
    impl.v4Index     = ipAdapter->IfIndex;
    impl.v6Index     = ipAdapter->Ipv6IfIndex;
    auto iter        = enableCache.find(ipAdapter->Description);
    impl.isEnable    = iter != enableCache.end() ? iter->second : false;
}

void FillAdapterInfo(NetAdapterImpl& impl, IP_ADAPTER_ADDRESSES* ipAdapter, const std::map<std::wstring, bool, WStringICaseKeyCompare>& enableCache)
{
    impl.id              = ipAdapter->AdapterName;
    impl.displayName     = CharsetUtils::UnicodeToUTF8(ipAdapter->FriendlyName);
    impl.deviceName      = CharsetUtils::UnicodeToUTF8(ipAdapter->Description);
    impl.physicalAddress = ParseAdapterMAC(ipAdapter->PhysicalAddress, kMACLength);
    impl.type            = CastAdapterType(ipAdapter->IfType);
    ParseIPAddress(ipAdapter->FirstUnicastAddress, impl.v4AddressList, impl.v6AddressList);
    ParseGateway(ipAdapter->FirstGatewayAddress, impl.v4GatewayList, impl.v6GatewayList);
    impl.speed       = ipAdapter->TransmitLinkSpeed;
    impl.isVirtual   = IsVirtualAdapter(impl.id);
    impl.isConnected = ipAdapter->OperStatus == IfOperStatusUp;
    impl.v4Index     = ipAdapter->IfIndex;
    impl.v6Index     = ipAdapter->Ipv6IfIndex;
    auto iter        = enableCache.find(ipAdapter->Description);
    impl.isEnable    = iter != enableCache.end() ? iter->second : false;
}

} // namespace
std::vector<NetAdapter> NetAdapter::List(NetAdapterType type, NetVersion version)
{
    std::vector<NetAdapter>                              adapters;
    std::map<std::wstring, bool, WStringICaseKeyCompare> enableCache = GetDeviceMap();
    adapters.reserve(enableCache.size());
    ULONG family = 0;
    switch (version)
    {
    case NetVersion::V4:
        family = AF_INET;
        break;
    case NetVersion::V6:
        family = AF_INET6;
        break;
    default:
        family = AF_UNSPEC;
    }

    std::unique_ptr<uint8_t[]> data = GetAdaptersAddressesData(family);
    if (data)
    {
        const auto virtualAdapters = ScanVirtualAdapter();
        for (auto* ipAdapter = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(data.get()); ipAdapter; ipAdapter = ipAdapter->Next)
        {
            if (ipAdapter->PhysicalAddressLength != kMACLength)
            {
                continue;
            }
            if (ipAdapter->OperStatus == IfOperStatusNotPresent)
            {
                continue;
            }
            if (enableCache.find(ipAdapter->Description) == enableCache.end())
            {
                continue;
            }
            if (type != CastAdapterType(ipAdapter->IfType))
            {
                continue;
            }
            NetAdapter adapter;
            FillAdapterInfo(*adapter._impl, ipAdapter, virtualAdapters, enableCache);
            adapters.emplace_back(std::move(adapter));
        }
    }
    return adapters;
}

std::vector<NetAdapter> NetAdapter::ListAll()
{
    std::vector<NetAdapter>                              adapters;
    std::map<std::wstring, bool, WStringICaseKeyCompare> enableCache = GetDeviceMap();
    adapters.reserve(enableCache.size());

    std::unique_ptr<uint8_t[]> data = GetAdaptersAddressesData(AF_UNSPEC);
    if (data)
    {
        const auto virtualAdapters = ScanVirtualAdapter();
        for (auto* ipAdapter = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(data.get()); ipAdapter; ipAdapter = ipAdapter->Next)
        {
            if (ipAdapter->PhysicalAddressLength != kMACLength)
            {
                continue;
            }
            if (ipAdapter->OperStatus == IfOperStatusNotPresent)
            {
                continue;
            }
            if (enableCache.find(ipAdapter->Description) == enableCache.end())
            {
                continue;
            }
            NetAdapter adapter;
            FillAdapterInfo(*adapter._impl, ipAdapter, virtualAdapters, enableCache);
            adapters.emplace_back(std::move(adapter));
        }
    }
    return adapters;
}

std::optional<NetAdapter> NetAdapter::FindNetAdapterById(const std::string& id)
{
    std::optional<NetAdapter>                            adapter;
    std::map<std::wstring, bool, WStringICaseKeyCompare> enableCache = GetDeviceMap();

    std::unique_ptr<uint8_t[]> data = GetAdaptersAddressesData(AF_UNSPEC);
    if (data)
    {
        for (auto* ipAdapter = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(data.get()); ipAdapter; ipAdapter = ipAdapter->Next)
        {
            if (ipAdapter->PhysicalAddressLength != kMACLength)
            {
                continue;
            }
            if (ipAdapter->OperStatus == IfOperStatusNotPresent)
            {
                continue;
            }
            if (enableCache.find(ipAdapter->Description) == enableCache.end())
            {
                continue;
            }
            if (0 == std::strcmp(ipAdapter->AdapterName, id.c_str()))
            {
                adapter = NetAdapter();
                FillAdapterInfo(*adapter.value()._impl, ipAdapter, enableCache);
                break;
            }
        }
    }
    return adapter;
}

} // namespace zeus
#endif