#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "zeus/foundation/system/system_notify.h"
#include "zeus/foundation/container/callback_manager.hpp"
#include "zeus/foundation/time/relative_timer.h"
#include "zeus/foundation/container/concurrent_vector.hpp"
#include "zeus/foundation/container/case_map.hpp"

namespace zeus
{
struct SystemNotifyImpl
{
    CallbackManager<const std::string&, SystemNotify::StorageAction>             storageChangeCallbacks;
    CallbackManager<const std::vector<std::string>&, SystemNotify::DeviceAction> deviceChangeCallbacks;
    RelativeTimer                                                                notifyTimer {false};
    std::mutex                                                                   mutex;
    std::atomic<bool>                                                            run = false;
    ConcurrentVector<std::string>                                                devicePlugCache;
    ConcurrentVector<std::string>                                                deviceUnplugCache;
#ifdef _WIN32
    std::thread                                  windowThread;
    HDEVNOTIFY                                   deviceNotify = nullptr;
    HWND                                         window       = nullptr;
    std::set<std::string, StringICaseKeyCompare> currentStorages;
#endif
#ifdef __linux__
    std::thread mountMonitorThread;
    std::thread udevMonitorThread;
#endif
};
}