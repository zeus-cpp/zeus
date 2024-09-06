#include "zeus/foundation/system/system_notify.h"
#ifdef __linux__
#include <future>
#include <cstring>
#include <cassert>
#include <libmount/libmount.h>
#include <libudev.h>
#include "zeus/foundation/core/posix/eintr_wrapper.h"
#include "zeus/foundation/resource/auto_release.h"
#include "impl/system_notify_impl.h"

namespace zeus
{

void RunMountMonitor(SystemNotifyImpl* impl, std::shared_ptr<std::promise<bool>> promise)
{
    const char*     selfMountPath = "/proc/self/mountinfo";
    libmnt_monitor* mnt           = mnt_new_monitor();
    assert(mnt);
    AutoRelease mntRelease(
        [&mnt]()
        {
            if (mnt)
            {
                mnt_unref_monitor(mnt);
            }
        }
    );
    int ret = mnt_monitor_enable_kernel(mnt, true);
    if (0 != ret)
    {
        promise->set_value(false);
        return;
    }
    promise->set_value(true);
    libmnt_table* oldTable = mnt_new_table_from_file(selfMountPath);
    assert(oldTable);
    AutoRelease oldTableRelease(
        [&oldTable]()
        {
            if (oldTable)
            {
                mnt_unref_table(oldTable);
            }
        }
    );

    while (impl->run)
    {
        int wait = HANDLE_EINTR(mnt_monitor_wait(mnt, 10));
        if (0 == wait)
        {
            continue;
        }
        else if (wait < 0)
        {
            break;
        }
        const char* filename = nullptr;
        while (mnt_monitor_next_change(mnt, &filename, nullptr) == 0)
        {
            if (0 != strcmp(filename, selfMountPath))
            {
                continue;
            }
            libmnt_table*   newTable  = nullptr;
            libmnt_tabdiff* tableDiff = nullptr;
            libmnt_fs*      oldItem   = nullptr;
            libmnt_fs*      newItem   = nullptr;
            libmnt_iter*    iter      = nullptr;
            AutoRelease     autoRelease(
                [&newTable, &tableDiff, &oldItem, &newItem, &iter]()
                {
                    if (newTable)
                    {
                        mnt_unref_table(newTable);
                    }
                    if (tableDiff)
                    {
                        mnt_free_tabdiff(tableDiff);
                    }
                    if (oldItem)
                    {
                        mnt_unref_fs(oldItem);
                    }
                    if (newItem)
                    {
                        mnt_unref_fs(newItem);
                    }
                    if (iter)
                    {
                        mnt_free_iter(iter);
                    }
                }
            );

            newTable = mnt_new_table_from_file(filename);
            if (!newTable)
            {
                continue;
            }
            tableDiff = mnt_new_tabdiff();
            assert(tableDiff);
            int count = mnt_diff_tables(tableDiff, oldTable, newTable);
            if (count <= 0)
            {
                continue;
            }
            std::swap(oldTable, newTable);
            oldItem = mnt_new_fs();
            newItem = mnt_new_fs();
            iter    = mnt_new_iter(MNT_ITER_FORWARD);
            assert(oldItem);
            assert(newItem);
            assert(iter);
            int operate = 0;
            while (mnt_tabdiff_next_change(tableDiff, iter, &oldItem, &newItem, &operate) == 0)
            {
                if (MNT_TABDIFF_MOUNT == operate || MNT_TABDIFF_REMOUNT == operate)
                {
                    auto target = mnt_fs_get_target(newItem);
                    if (!target)
                    {
                        continue;
                    }
                    std::string path(target);
                    impl->notifyTimer.AddDelayTimerTask(
                        [impl, path]() { impl->storageChangeCallbacks.Call(path, SystemNotify::StorageAction::Mount); }, std::chrono::milliseconds(0)
                    );
                }
                else if (MNT_TABDIFF_UMOUNT == operate || MNT_TABDIFF_MOVE == operate)
                {
                    auto target = mnt_fs_get_target(oldItem);
                    if (!target)
                    {
                        continue;
                    }
                    std::string path(target);
                    impl->notifyTimer.AddDelayTimerTask(
                        [impl, path]() { impl->storageChangeCallbacks.Call(path, SystemNotify::StorageAction::Unmount); },
                        std::chrono::milliseconds(0)
                    );
                }
            }
        }
    }
}

void RunUdevMonitor(SystemNotifyImpl* impl, std::shared_ptr<std::promise<bool>> promise)
{
    impl->devicePlugCache.Reserve(16);
    impl->deviceUnplugCache.Reserve(16);
    impl->devicePlugCache.Clear();
    impl->deviceUnplugCache.Clear();
    udev* udev = udev_new();
    if (!udev)
    {
        promise->set_value(false);
        return;
    }
    AutoRelease udevRelease(
        [&udev]()
        {
            if (udev)
            {
                udev_unref(udev);
            }
        }
    );
    udev_monitor* monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!monitor)
    {
        promise->set_value(false);
        return;
    }
    promise->set_value(true);
    AutoRelease monitorRelease(
        [&monitor]()
        {
            if (monitor)
            {
                udev_monitor_unref(monitor);
            }
        }
    );

    udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", "usb_device");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "video4linux", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "sound", nullptr);
    udev_monitor_enable_receiving(monitor);
    auto fd = udev_monitor_get_fd(monitor);
    while (impl->run)
    {
        fd_set         fds;
        struct timeval tv;
        int            ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec  = 0;
        tv.tv_usec = 1000 * 10;

        ret = select(fd + 1, &fds, nullptr, nullptr, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds))
        {
            udev_device* dev = udev_monitor_receive_device(monitor);
            if (dev)
            {
                AutoRelease devRelease(
                    [&dev]()
                    {
                        if (dev)
                        {
                            udev_device_unref(dev);
                        }
                    }
                );
                auto devpath = udev_device_get_devpath(dev);
                if (!devpath)
                {
                    continue;
                }
                auto action = udev_device_get_action(dev);
                if (!action)
                {
                    continue;
                }
                SystemNotify::DeviceAction devAction = SystemNotify::DeviceAction::Unknown;
                if (0 == strcmp("add", action))
                {
                    devAction = SystemNotify::DeviceAction::Plug;
                }
                else if (0 == strcmp("remove", action))
                {
                    devAction = SystemNotify::DeviceAction::Unplug;
                }
                switch (devAction)
                {
                case SystemNotify::DeviceAction::Plug:
                    impl->devicePlugCache.EmplaceBack(devpath);
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
                        std::chrono::milliseconds(300)
                    );
                    break;
                case SystemNotify::DeviceAction::Unplug:
                    impl->deviceUnplugCache.EmplaceBack(devpath);
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
                        std::chrono::milliseconds(300)
                    );
                    break;
                default:
                    continue;
                }
            }
        }
    }
}

bool SystemNotify::Start()
{
    std::unique_lock lock(_impl->mutex);
    if (!_impl->run)
    {
        _impl->run = true;
        _impl->notifyTimer.Start();
        auto mountPromise         = std::make_shared<std::promise<bool>>();
        auto devPromise           = std::make_shared<std::promise<bool>>();
        _impl->mountMonitorThread = std::thread(RunMountMonitor, _impl.get(), mountPromise);
        _impl->udevMonitorThread  = std::thread(RunUdevMonitor, _impl.get(), devPromise);
        const auto result         = mountPromise->get_future().get() && devPromise->get_future().get();
        if (!result)
        {
            _impl->run = false;
            if (_impl->mountMonitorThread.joinable())
            {
                _impl->mountMonitorThread.join();
            }
            if (_impl->udevMonitorThread.joinable())
            {
                _impl->udevMonitorThread.join();
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
        if (_impl->mountMonitorThread.joinable())
        {
            _impl->mountMonitorThread.join();
        }
        if (_impl->udevMonitorThread.joinable())
        {
            _impl->udevMonitorThread.join();
        }
        _impl->notifyTimer.Stop();
    }
    return;
}

} // namespace zeus
#endif