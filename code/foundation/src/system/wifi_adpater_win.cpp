#include "zeus/foundation/system/wifi_adpater.h"
#ifdef _WIN32
#include <map>
#include <Windows.h>
#include <wlanapi.h>

#include <algorithm>

#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/crypt/uuid.h"
#include <fmt/format.h>
#include "impl/wifi_adpater_impl.h"

#pragma comment(lib, "wlanapi.lib")

using namespace std;

namespace zeus
{
WifiAdpater::WifiAdpater() : _impl(std::make_unique<WifiAdpaterImpl>())
{
}

WifiAdpater::~WifiAdpater()
{
}
WifiAdpater::WifiAdpater(const WifiAdpater& other)
{
    if (other._impl)
    {
        _impl = std::make_unique<WifiAdpaterImpl>(*other._impl);
    }
    else
    {
        _impl.reset();
    }
}
WifiAdpater::WifiAdpater(WifiAdpater&& other)
{
    _impl.swap(other._impl);
}

std::vector<WifiAp> WifiAdpater::ListAp() const
{
    std::vector<WifiAp>         result;
    HANDLE                      wlanHandle  = nullptr;
    PWLAN_BSS_LIST              bssList     = nullptr;
    WLAN_CONNECTION_ATTRIBUTES* connectInfo = nullptr;
    AutoRelease                 raii(
        [&]()
        {
            if (wlanHandle)
            {
                WlanCloseHandle(wlanHandle, nullptr);
            }
            if (bssList)
            {
                WlanFreeMemory(bssList);
            }
            if (connectInfo)
            {
                WlanFreeMemory(connectInfo);
            }
        }
    );
    DWORD supportVersion = 0;
    if (ERROR_SUCCESS == WlanOpenHandle(2, NULL, &supportVersion, &wlanHandle))
    {
        if (ERROR_SUCCESS == WlanGetNetworkBssList(wlanHandle, &_impl->guid, nullptr, dot11_BSS_type_any, FALSE, nullptr, &bssList))
        {
            std::array<uint8_t, 6> connectedMac = {0};
            string                 connectedSsid;
            DWORD                  length = sizeof(WLAN_CONNECTION_ATTRIBUTES);

            if (ERROR_SUCCESS ==
                WlanQueryInterface(
                    wlanHandle, &_impl->guid, wlan_intf_opcode_current_connection, nullptr, &length, reinterpret_cast<void**>(&connectInfo), nullptr
                ))
            {
                if (connectInfo->isState == wlan_interface_state_connected && connectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength)
                {
                    std::memcpy(connectedMac.data(), connectInfo->wlanAssociationAttributes.dot11Bssid, sizeof(connectedMac));
                    connectedSsid = string(
                        connectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID,
                        connectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID + connectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength
                    );
                }
            }
            std::map<std::string, std::vector<WifiBssImpl>> cache;
            for (auto i = 0U; i < bssList->dwNumberOfItems; i++)
            {
                auto& bss = bssList->wlanBssEntries[i];
                if (bss.dot11Ssid.uSSIDLength)
                {
                    if (!std::all_of(
                            bss.dot11Ssid.ucSSID, bss.dot11Ssid.ucSSID + bss.dot11Ssid.uSSIDLength, [](const uint8_t& item) { return 0 == item; }
                        ))
                    {
                        auto ssid = std::string(bss.dot11Ssid.ucSSID, bss.dot11Ssid.ucSSID + bss.dot11Ssid.uSSIDLength);
                        if (ssid.empty())
                        {
                            continue;
                        }
                        auto bssItem = WifiBssImpl();
                        if (ssid == connectedSsid && 0 == std::memcmp(bss.dot11Bssid, connectedMac.data(), sizeof(connectedMac)))
                        {
                            bssItem.connected = true;
                        }
                        else
                        {
                            bssItem.connected = false;
                        }
                        std::memcpy(bssItem.mac.data(), bss.dot11Bssid, sizeof(bss.dot11Bssid));
                        bssItem.ssid          = ssid;
                        bssItem.signalQuality = bss.uLinkQuality;
                        bssItem.rssi          = bss.lRssi;
                        bssItem.frequency     = bss.ulChCenterFrequency;
                        cache[ssid].emplace_back(std::move(bssItem));
                    }
                }
            }

            result.reserve(cache.size());
            for (auto& iter : cache)
            {
                WifiAp temp;
                result.emplace_back(temp);
                auto& ap       = result.back();
                ap._impl->ssid = iter.first;
                for (auto& bss : iter.second)
                {
                    ap._impl->bssList.emplace_back(std::move(bss));
                }
            }
        }
    }

    return result;
}

std::string WifiAdpater::Id() const
{
    return _impl->id;
}
std::string WifiAdpater::Name() const
{
    return _impl->name;
}
std::shared_ptr<WifiBss> WifiAdpater::GetConnectedBss() const
{
    std::shared_ptr<WifiBss>    result;
    HANDLE                      wlanHandle  = nullptr;
    PWLAN_BSS_LIST              bssList     = nullptr;
    WLAN_CONNECTION_ATTRIBUTES* connectInfo = nullptr;
    AutoRelease                 raii(
        [&]()
        {
            if (wlanHandle)
            {
                WlanCloseHandle(wlanHandle, nullptr);
            }
            if (bssList)
            {
                WlanFreeMemory(bssList);
            }
            if (connectInfo)
            {
                WlanFreeMemory(connectInfo);
            }
        }
    );
    DWORD supportVersion = 0;
    if (ERROR_SUCCESS == WlanOpenHandle(2, NULL, &supportVersion, &wlanHandle))
    {
        DWORD length = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        if (ERROR_SUCCESS ==
            WlanQueryInterface(
                wlanHandle, &_impl->guid, wlan_intf_opcode_current_connection, nullptr, &length, reinterpret_cast<void**>(&connectInfo), nullptr
            ))
        {
            if (connectInfo->isState == wlan_interface_state_connected && connectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength)
            {
                WifiBss temp;
                result = std::make_shared<WifiBss>(std::move(temp));
                std::memcpy(result->_impl->mac.data(), connectInfo->wlanAssociationAttributes.dot11Bssid, sizeof(result->_impl->mac));
                result->_impl->ssid = string(
                    connectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID,
                    connectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID + connectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength
                );
                result->_impl->signalQuality        = connectInfo->wlanAssociationAttributes.wlanSignalQuality;
                result->_impl->connected            = true;
                result->_impl->rssi                 = -100 + result->_impl->signalQuality / 2;
                std::array<uint8_t, 6> connectedMac = {0};
                string                 connectedSsid;
                std::memcpy(connectedMac.data(), connectInfo->wlanAssociationAttributes.dot11Bssid, sizeof(connectedMac));
                if (ERROR_SUCCESS == WlanGetNetworkBssList(
                                         wlanHandle, &_impl->guid, &connectInfo->wlanAssociationAttributes.dot11Ssid,
                                         connectInfo->wlanAssociationAttributes.dot11BssType, TRUE, nullptr, &bssList
                                     ))
                {
                    for (auto i = 0U; i < bssList->dwNumberOfItems; i++)
                    {
                        auto& bss = bssList->wlanBssEntries[i];
                        if (bss.dot11Ssid.uSSIDLength && 0 == std::memcmp(bss.dot11Bssid, connectedMac.data(), sizeof(connectedMac)))
                        {
                            result->_impl->rssi      = bss.lRssi;
                            result->_impl->frequency = bss.ulChCenterFrequency;
                        }
                    }
                }
                if (bssList)
                {
                    WlanFreeMemory(bssList);
                    bssList = nullptr;
                }
                if (ERROR_SUCCESS == WlanGetNetworkBssList(
                                         wlanHandle, &_impl->guid, &connectInfo->wlanAssociationAttributes.dot11Ssid,
                                         connectInfo->wlanAssociationAttributes.dot11BssType, FALSE, nullptr, &bssList
                                     ))
                {
                    for (auto i = 0U; i < bssList->dwNumberOfItems; i++)
                    {
                        auto& bss = bssList->wlanBssEntries[i];
                        if (bss.dot11Ssid.uSSIDLength && 0 == std::memcmp(bss.dot11Bssid, connectedMac.data(), sizeof(connectedMac)))
                        {
                            result->_impl->rssi      = bss.lRssi;
                            result->_impl->frequency = bss.ulChCenterFrequency;
                        }
                    }
                }
                if (bssList)
                {
                    WlanFreeMemory(bssList);
                    bssList = nullptr;
                }
            }
        }
    }
    return result;
}
bool WifiAdpater::IsConnected() const
{
    return _impl->connected;
}
bool WifiAdpater::Scan()
{
    HANDLE      wlanHandle = nullptr;
    AutoRelease raii(
        [&]()
        {
            if (wlanHandle)
            {
                WlanCloseHandle(wlanHandle, nullptr);
            }
        }
    );
    DWORD supportVersion = 0;
    if (ERROR_SUCCESS == WlanOpenHandle(2, NULL, &supportVersion, &wlanHandle))
    {
        if (ERROR_SUCCESS == WlanScan(wlanHandle, &_impl->guid, nullptr, nullptr, nullptr))
        {
            return true;
        }
    }

    return false;
}

std::vector<WifiAdpater> WifiAdpater::List()
{
    std::vector<WifiAdpater>  result;
    HANDLE                    wlanHandle = nullptr;
    PWLAN_INTERFACE_INFO_LIST interfaces = nullptr;
    AutoRelease               raii(
        [&]()
        {
            if (wlanHandle)
            {
                WlanCloseHandle(wlanHandle, nullptr);
            }
            if (interfaces)
            {
                WlanFreeMemory(interfaces);
            }
        }
    );
    DWORD supportVersion = 0;
    if (ERROR_SUCCESS == WlanOpenHandle(2, NULL, &supportVersion, &wlanHandle))
    {
        if (ERROR_SUCCESS == WlanEnumInterfaces(wlanHandle, nullptr, &interfaces))
        {
            for (auto i = 0U; i < interfaces->dwNumberOfItems; i++)
            {
                auto&       info = interfaces->InterfaceInfo[i];
                WifiAdpater temp;
                result.emplace_back(temp);
                auto& adpater            = result.back();
                adpater._impl->name      = CharsetUtils::UnicodeToUTF8(info.strInterfaceDescription);
                adpater._impl->id        = Uuid::toString(info.InterfaceGuid);
                adpater._impl->connected = info.isState == WLAN_INTERFACE_STATE::wlan_interface_state_connected;
                adpater._impl->guid      = info.InterfaceGuid;
            }
        }
    }
    return result;
}

WifiAp::WifiAp() : _impl(std::make_unique<WifiApImpl>())
{
}

WifiAp::~WifiAp()
{
}
WifiAp::WifiAp(const WifiAp& other)
{
    if (other._impl)
    {
        _impl = std::make_unique<WifiApImpl>(*other._impl);
    }
    else
    {
        _impl.reset();
    }
}
WifiAp::WifiAp(WifiAp&& other)
{
    _impl.swap(other._impl);
}
WifiAp& WifiAp::operator=(const WifiAp& other)
{
    if (&other != this)
    {
        if (other._impl)
        {
            _impl = std::make_unique<WifiApImpl>(*other._impl);
        }
        else
        {
            _impl.reset();
        }
    }
    return *this;
}
WifiAp& WifiAp::operator=(WifiAp&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string WifiAp::SSID() const
{
    return _impl->ssid;
}
uint8_t WifiAp::MaxSignalQuality() const
{
    uint8_t max = 0;
    for (auto& bss : _impl->bssList)
    {
        if (bss.signalQuality > max)
        {
            max = bss.signalQuality;
        }
    }
    return max;
}
std::vector<WifiBss> WifiAp::BssList() const
{
    std::vector<WifiBss> result;
    for (auto& bss : _impl->bssList)
    {
        WifiBss temp;
        temp._impl = std::make_unique<WifiBssImpl>(bss);
        result.emplace_back(std::move(temp));
    }
    return result;
}
uint8_t WifiBss::SignalQuality() const
{
    return _impl->signalQuality;
}
size_t WifiBss::Frequency()
{
    return _impl->frequency;
}
int32_t WifiBss::CastSignalQualityToRssi(uint8_t quality)
{
    return -100 + quality / 2;
}
WifiBss::WifiStrength WifiBss::CastSignalQualityToStrength(uint8_t quality)
{
    return CastRssiToStrength(CastSignalQualityToRssi(quality));
}
WifiBss::WifiStrength WifiBss::CastRssiToStrength(int32_t rssi)
{
    if (rssi < -70)
    {
        return WifiBss::WifiStrength::Weak;
    }
    if (rssi <= -60)
    {
        return WifiBss::WifiStrength::Fair;
    }
    if (rssi <= -50)
    {
        return WifiBss::WifiStrength::Good;
    }
    if (rssi > -50)
    {
        return WifiBss::WifiStrength::Excellent;
    }
    else
    {
        return WifiBss::WifiStrength::Weak;
    }
}
bool WifiBss::IsConnected() const
{
    return _impl->connected;
}
int32_t WifiBss::Rssi() const
{
    return _impl->rssi;
}

WifiBss::WifiStrength WifiBss::Strength() const
{
    return CastRssiToStrength(_impl->rssi);
}

WifiBss::WifiBss() : _impl(std::make_unique<WifiBssImpl>())
{
}

WifiBss::~WifiBss()
{
}

WifiBss::WifiBss(const WifiBss& other)
{
    if (&other != this)
    {
        if (other._impl)
        {
            _impl = std::make_unique<WifiBssImpl>(*other._impl);
        }
        else
        {
            _impl.reset();
        }
    }
}

WifiBss::WifiBss(WifiBss&& other)
{
    _impl.swap(other._impl);
}

WifiBss& WifiBss::operator=(const WifiBss& other)
{
    if (&other != this)
    {
        if (other._impl)
        {
            _impl = std::make_unique<WifiBssImpl>(*other._impl);
        }
        else
        {
            _impl.reset();
        }
    }
    return *this;
}

WifiBss& WifiBss::operator=(WifiBss&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

std::string WifiBss::Mac(const std::string& split) const
{
    std::string mac;
    mac = fmt::format(
        ("{:02X}" + split + "{:02X}" + split + "{:02X}" + split + "{:02X}" + split + "{:02X}" + split + "{:02X}"), _impl->mac[0], _impl->mac[1],
        _impl->mac[2], _impl->mac[3], _impl->mac[4], _impl->mac[5]
    );
    return mac;
}

std::string WifiBss::SSID() const
{
    return _impl->ssid;
}

WifiChangeNotify::WifiChangeNotify() : _impl(std::make_unique<WifiChangeNotifyImpl>())
{
}
WifiChangeNotify::~WifiChangeNotify()
{
    Stop();
}

void WifiChangeNotify::AddSignChangeCallback(const std::function<void(const WifiAdpater& adpater, size_t signal)>& callback)
{
    _impl->_signCallback.PushBack(callback);
}

void WifiChangeNotify::AddConnectCallback(const std::function<void(const WifiAdpater& adpater, const std::string& ssid, bool state)>& callback)
{
    _impl->_connectCallback.PushBack(callback);
}

void WifiChangeNotify::AddDeviceCallback(const std::function<void(const WifiAdpater& adpater, bool state)>& callback)
{
    _impl->_deviceCallback.PushBack(callback);
}

void WifiChangeNotify::AddScanCallback(const std::function<void(const WifiAdpater& adpater)>& callback)
{
    _impl->_scanCallback.PushBack(callback);
}

bool WifiChangeNotify::Start()
{
    DWORD supportVersion = 0;
    if (ERROR_SUCCESS == WlanOpenHandle(2, NULL, &supportVersion, &_impl->wlanHandle))
    {
        DWORD temp;
        if (ERROR_SUCCESS == WlanRegisterNotification(
                                 _impl->wlanHandle, WLAN_NOTIFICATION_SOURCE_ALL, TRUE,
                                 reinterpret_cast<WLAN_NOTIFICATION_CALLBACK>(WifiChangeNotifyImpl::WifiNotify), _impl.get(), nullptr, &temp
                             ))
        {
            return true;
        }
    }

    return false;
}
void WifiChangeNotify::Stop()
{
    if (_impl->wlanHandle)
    {
        DWORD temp;
        WlanRegisterNotification(_impl->wlanHandle, WLAN_NOTIFICATION_SOURCE_NONE, FALSE, nullptr, nullptr, nullptr, &temp);
        WlanCloseHandle(_impl->wlanHandle, nullptr);
    }
}

void WifiChangeNotifyImpl::WifiNotify(PWLAN_NOTIFICATION_DATA data, WifiChangeNotifyImpl* context)
{
    if (data->NotificationSource == WLAN_NOTIFICATION_SOURCE_ACM)
    {
        WLAN_CONNECTION_NOTIFICATION_DATA* connection;
        switch (data->NotificationCode)
        {
        case wlan_notification_acm_connection_complete:
            connection = reinterpret_cast<WLAN_CONNECTION_NOTIFICATION_DATA*>(data->pData);
            if (WLAN_REASON_CODE_SUCCESS == connection->wlanReasonCode)
            {
                context->NotifyConnectChange(
                    Uuid::toString(data->InterfaceGuid),
                    std::string(connection->dot11Ssid.ucSSID, connection->dot11Ssid.ucSSID + connection->dot11Ssid.uSSIDLength), true
                );
            }
            break;
        case wlan_notification_acm_disconnected:
            connection = reinterpret_cast<WLAN_CONNECTION_NOTIFICATION_DATA*>(data->pData);
            context->NotifyConnectChange(
                Uuid::toString(data->InterfaceGuid),
                std::string(connection->dot11Ssid.ucSSID, connection->dot11Ssid.ucSSID + connection->dot11Ssid.uSSIDLength), false
            );
            break;
        case wlan_notification_acm_interface_arrival:
            context->NotifyDeviceChange(Uuid::toString(data->InterfaceGuid), true);
            break;
        case wlan_notification_acm_interface_removal:
            context->NotifyDeviceChange(Uuid::toString(data->InterfaceGuid), false);
            break;
        case wlan_notification_acm_scan_complete:
            context->NofityScanComplete(Uuid::toString(data->InterfaceGuid));
            break;
        default:
            break;
        }
    }
    if (data->NotificationSource == WLAN_NOTIFICATION_SOURCE_MSM)
    {
        switch (data->NotificationCode)
        {
        case wlan_notification_msm_signal_quality_change:
            context->NotifySignalChange(Uuid::toString(data->InterfaceGuid), *reinterpret_cast<ULONG*>(data->pData));
            break;
        default:
            break;
        }
    }
}

void WifiChangeNotifyImpl::NotifyConnectChange(const std::string& guid, const std::string& ssid, bool state)
{
    std::shared_ptr<WifiAdpater> adpater = Cache(guid);
    if (!adpater)
    {
        return;
    }
    auto callbacks = _connectCallback.Data();
    for (auto& callback : callbacks)
    {
        callback->operator()(*adpater, ssid, state);
    }
}

void WifiChangeNotifyImpl::NotifyDeviceChange(const std::string& guid, bool state)
{
    std::shared_ptr<WifiAdpater> adpater = Cache(guid);
    if (!adpater)
    {
        return;
    }
    auto callbacks = _deviceCallback.Data();
    for (auto& callback : callbacks)
    {
        callback->operator()(*adpater, state);
    }
}

void WifiChangeNotifyImpl::NofityScanComplete(const std::string& guid)
{
    std::shared_ptr<WifiAdpater> adpater = Cache(guid);
    if (!adpater)
    {
        return;
    }
    auto callbacks = _scanCallback.Data();
    for (auto& callback : callbacks)
    {
        callback->operator()(*adpater);
    }
}

std::shared_ptr<WifiAdpater> WifiChangeNotifyImpl::Cache(const std::string& guid)
{
    auto iter = cache.find(guid);
    if (iter != cache.end())
    {
        return iter->second.first;
    }
    else
    {
        auto adpaters = WifiAdpater::List();
        auto item     = std::find_if(adpaters.begin(), adpaters.end(), [&guid](const WifiAdpater& item) { return item.Id() == guid; });
        if (item != adpaters.end())
        {
            auto adpater = std::make_shared<WifiAdpater>(*item);
            cache[guid]  = std::make_pair(adpater, 0);
            return adpater;
        }
        else
        {
            return nullptr;
        }
    }
}

void WifiChangeNotifyImpl::NotifySignalChange(const string& guid, size_t signal)
{
    std::shared_ptr<WifiAdpater> adpater = Cache(guid);
    if (!adpater)
    {
        return;
    }
    auto& oldSignal = cache[guid].second;
    if (signal == oldSignal)
    {
        return;
    }
    oldSignal = signal;

    auto callbacks = _signCallback.Data();
    for (auto& callback : callbacks)
    {
        callback->operator()(*adpater, signal);
    }
}
} // namespace zeus
#endif
