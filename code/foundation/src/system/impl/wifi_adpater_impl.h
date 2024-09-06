#pragma once
#ifdef _WIN32
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <Guiddef.h>
#include "zeus/foundation/container/concurrent_list.hpp"
namespace zeus
{
struct WifiBssImpl
{
    std::string            ssid;
    uint32_t               signalQuality;
    int32_t                rssi;
    bool                   connected;
    std::array<uint8_t, 6> mac;
    size_t                 frequency;
};

struct WifiApImpl
{
    std::string              ssid;
    std::vector<WifiBssImpl> bssList;
};

struct WifiAdpaterImpl
{
    std::string name;
    std::string id;
    bool        connected;
    GUID        guid;
};

struct WifiChangeNotifyImpl
{
    HANDLE                                                                                                     wlanHandle = nullptr;
    ConcurrentList<std::function<void(const WifiAdpater& adpater, const std::string& ssid, bool state)>, true> _connectCallback;
    ConcurrentList<std::function<void(const WifiAdpater& adpater, bool state)>, true>                          _deviceCallback;
    ConcurrentList<std::function<void(const WifiAdpater& adpater)>, true>                                      _scanCallback;
    ConcurrentList<std::function<void(const WifiAdpater& adpater, size_t signal)>, true>                       _signCallback;
    std::map<std::string, std::pair<std::shared_ptr<WifiAdpater>, size_t>>                                     cache;
    static void                  WifiNotify(PWLAN_NOTIFICATION_DATA data, WifiChangeNotifyImpl* context);
    void                         NotifySignalChange(const std::string& guid, size_t signal);
    void                         NotifyConnectChange(const std::string& guid, const std::string& ssid, bool state);
    void                         NotifyDeviceChange(const std::string& guid, bool state);
    void                         NofityScanComplete(const std::string& guid);
    std::shared_ptr<WifiAdpater> Cache(const std::string& guid);
};
} // namespace zeus
#endif
