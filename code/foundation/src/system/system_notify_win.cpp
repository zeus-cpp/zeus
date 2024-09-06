#include "zeus/foundation/system/system_notify.h"
#ifdef _WIN32
#include <list>
#include <future>
#include <Windows.h>
#include <dbt.h>
#include <wtsapi32.h>
#include "zeus/foundation/hardware/storage.h"
#include "zeus/foundation/crypt/uuid.h"
#include "zeus/foundation/string/charset_utils.h"
#include "impl/system_notify_impl.h"

namespace zeus
{
std::chrono::milliseconds deviceChangeCacheDuration(300);

std::list<std::string> CastVolumePath(DWORD dwData)
{
    std::list<std::string> volumes;
    for (char offset = 0; offset < 26; ++offset)
    {
        if (dwData & 0x1)
        {
            std::string path;
            path.push_back(offset + 'A');
            path.push_back(':');
            volumes.emplace_back(path);
        }
        dwData = dwData >> 1;
        if (dwData == 0x0000)
        {
            break;
        }
    }
    return volumes;
}

INT_PTR WINAPI WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* impl = reinterpret_cast<SystemNotifyImpl*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (message == WM_CREATE)
    {
        auto* data = reinterpret_cast<CREATESTRUCTA*>(lParam);
        impl       = reinterpret_cast<SystemNotifyImpl*>(data->lpCreateParams);
    }
    if (impl)
    {
        switch (message)
        {
        case WM_CREATE:
        {
            DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
            ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
            NotificationFilter.dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

            impl->deviceNotify =
                RegisterDeviceNotificationW(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

            auto storages = zeus::Hardware::Storage::ListAll();
            for (const auto& storage : storages)
            {
                impl->currentStorages.emplace(storage.Path());
            }
            WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_ALL_SESSIONS);
        }
        break;

        case WM_DEVICECHANGE:
            if (DBT_DEVICEARRIVAL == wParam)
            {
                auto* pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
                if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    std::string devName = zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE_W>(pHdr)->dbcc_name);
                    impl->devicePlugCache.EmplaceBack(devName);
                    impl->notifyTimer.AddDelayTimerTask(
                        [impl]()
                        {
                            decltype(impl->devicePlugCache)::DataType plugData;
                            impl->devicePlugCache.Swap(plugData);
                            if (!plugData.empty())
                            {
                                impl->deviceChangeCallbacks.Call(plugData, SystemNotify::DeviceAction::Plug);
                            }
                        },
                        deviceChangeCacheDuration
                    );
                }
                if (DBT_DEVTYP_VOLUME == pHdr->dbch_devicetype)
                {
                    auto volumes = CastVolumePath(reinterpret_cast<const PDEV_BROADCAST_VOLUME>(pHdr)->dbcv_unitmask);
                    for (const auto& volume : volumes)
                    {
                        impl->currentStorages.emplace(volume);
                    }
                    impl->notifyTimer.AddDelayTimerTask(
                        [impl, volumes]()
                        {
                            for (const auto& volume : volumes)
                            {
                                impl->storageChangeCallbacks.Call(volume, SystemNotify::StorageAction::Mount);
                            }
                        },
                        std::chrono::milliseconds(0)
                    );
                }
            }
            else if (DBT_DEVICEREMOVECOMPLETE == wParam)
            {
                auto* pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
                if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    std::string devName = zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE_W>(pHdr)->dbcc_name);
                    impl->deviceUnplugCache.EmplaceBack(devName);
                    impl->notifyTimer.AddDelayTimerTask(
                        [impl]()
                        {
                            decltype(impl->deviceUnplugCache)::DataType unplugData;
                            impl->deviceUnplugCache.Swap(unplugData);
                            if (!unplugData.empty())
                            {
                                impl->deviceChangeCallbacks.Call(unplugData, SystemNotify::DeviceAction::Unplug);
                            }
                        },
                        deviceChangeCacheDuration
                    );
                }
                if (DBT_DEVTYP_VOLUME == pHdr->dbch_devicetype)
                {
                    auto volumes = CastVolumePath(reinterpret_cast<PDEV_BROADCAST_VOLUME>(pHdr)->dbcv_unitmask);
                    for (auto volumeIter = volumes.begin(); volumeIter != volumes.end();)
                    {
                        if (auto storageIter = impl->currentStorages.find(*volumeIter); storageIter != impl->currentStorages.end())
                        {
                            impl->currentStorages.erase(storageIter);
                            ++volumeIter;
                        }
                        else
                        {
                            volumeIter = volumes.erase(volumeIter);
                        }
                    }
                    impl->notifyTimer.AddDelayTimerTask(
                        [impl, volumes]()
                        {
                            for (const auto& volume : volumes)
                            {
                                impl->storageChangeCallbacks.Call(volume, SystemNotify::StorageAction::Unmount);
                            }
                        },
                        std::chrono::milliseconds(0)
                    );
                }
            }
            break;

        case WM_CLOSE:
            UnregisterDeviceNotification(impl->deviceNotify);
            WTSUnRegisterSessionNotification(hWnd);
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    else
    {
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RunWindowMonitor(SystemNotifyImpl* impl, std::shared_ptr<std::promise<bool>> promise)
{
    WNDCLASSEXW wndClass;

    wndClass.cbSize        = sizeof(WNDCLASSEXW);
    wndClass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance     = reinterpret_cast<HINSTANCE>(GetModuleHandle(nullptr));
    wndClass.lpfnWndProc   = reinterpret_cast<WNDPROC>(WinProcCallback);
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hIcon         = nullptr;
    wndClass.hbrBackground = nullptr;
    wndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);

    std::wstring className = CharsetUtils::UTF8ToUnicode(Uuid::GenerateRandom().toString());
    wndClass.lpszClassName = className.c_str();
    wndClass.lpszMenuName  = nullptr;
    wndClass.hIconSm       = nullptr;

    if (!RegisterClassEx(&wndClass))
    {
        promise->set_value(false);
        return;
    }

    impl->window = CreateWindowExW(
        WS_EX_NOACTIVATE, // 该参数保证该窗口不被激活
        className.c_str(), nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, 0, impl
    );
    if (nullptr == impl->window)
    {
        promise->set_value(false);
        return;
    }
    promise->set_value(true);
    SetWindowLongPtr(impl->window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(impl));
    // Actually draw the window.
    ShowWindow(impl->window, SW_HIDE);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool SystemNotify::Start()
{
    std::unique_lock lock(_impl->mutex);
    if (!_impl->run)
    {
        _impl->run = true;
        _impl->notifyTimer.Start();
        auto promise        = std::make_shared<std::promise<bool>>();
        _impl->windowThread = std::thread(RunWindowMonitor, _impl.get(), promise);
        const auto result   = promise->get_future().get();
        if (!result)
        {
            _impl->run = false;
            if (_impl->window)
            {
                PostMessageW(_impl->window, WM_CLOSE, 0, 0);
            }
            if (_impl->windowThread.joinable())
            {
                _impl->windowThread.join();
            }
            _impl->notifyTimer.Stop();
        }
        return result;
    }
    return false;
}
void SystemNotify::Stop()
{
    std::unique_lock lock(_impl->mutex);
    if (_impl->run)
    {
        _impl->run = false;

        if (_impl->window)
        {
            PostMessageW(_impl->window, WM_CLOSE, 0, 0);
        }
        if (_impl->windowThread.joinable())
        {
            _impl->windowThread.join();
        }
        _impl->notifyTimer.Stop();
        return;
    }
}

} // namespace zeus
#endif