#include "zeus/foundation/system/net_adapter_notify.h"
#ifdef __linux__
#include <map>
#include <mutex>
#include <filesystem>
#include <cassert>
#include <cstring>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include "zeus/foundation/core/posix/eintr_wrapper.h"
#include "zeus/foundation/sync/linux/file_descriptor_set_notify.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/container/callback_manager.hpp"

namespace zeus
{
struct NetAdapterNotifyImpl
{
    LinuxFileDescriptor                                                 fileDescription;
    LinuxFileDescriptorSetNotify                                        notify {false};
    bool                                                                run = false;
    std::mutex                                                          mutex;
    std::map<std::string, NetAdapterNotify::ConnectState>               cache;
    CallbackManager<const std::string&, NetAdapterNotify::ConnectState> callbacks;
    size_t                                                              notifyId = 0;
};
namespace
{

NetAdapterNotify::ConnectState CastState(const ifinfomsg* msg)
{
    if (msg->ifi_flags & IFF_RUNNING && msg->ifi_flags & IFF_UP)
    {
        return NetAdapterNotify::ConnectState::kConnected;
    }
    else
    {
        return NetAdapterNotify::ConnectState::kDisconnected;
    }
}

bool IgnoreWirelessChange(const ifinfomsg* msg, int length)
{
    for (const struct rtattr* attr = IFLA_RTA(msg); RTA_OK(attr, length); attr = RTA_NEXT(attr, length))
    {
        if (attr->rta_type == IFLA_WIRELESS && msg->ifi_change == 0)
        {
            return true;
        }
    }
    return false;
}

template<typename T>
T* SafelyCastNetlinkMsgData(const struct nlmsghdr* header, int length)
{
    assert(NLMSG_OK(header, static_cast<uint32_t>(length)));
    if (length <= 0 || static_cast<size_t>(length) < NLMSG_HDRLEN + sizeof(T))
    {
        return nullptr;
    }
    return reinterpret_cast<const T*>(NLMSG_DATA(header));
}

std::string GetAdapterId(const struct ifinfomsg* msg, int length)
{
    for (const struct rtattr* attr = IFLA_RTA(msg); RTA_OK(attr, length); attr = RTA_NEXT(attr, length))
    {
        if (IFLA_IFNAME == attr->rta_type && attr)
        {
            return reinterpret_cast<const char*>(RTA_DATA(attr));
        }
    }
    return std::string {};
}

void NetlinkChangeNotify(NetAdapterNotifyImpl& impl)
{
    char buffer[4096];
    while (true)
    {
        int length = HANDLE_EINTR(recv(impl.fileDescription.Fd(), buffer, sizeof(buffer), MSG_DONTWAIT));

        if (length > 0)
        {
            for (const nlmsghdr* header = reinterpret_cast<const nlmsghdr*>(buffer); NLMSG_OK(header, static_cast<uint32_t>(length));
                 header                 = NLMSG_NEXT(header, length))
            {
                switch (header->nlmsg_type)
                {
                case NLMSG_DONE:
                    break;
                case NLMSG_ERROR:
                    break;
                case RTM_NEWLINK:
                {
                    const struct ifinfomsg* msg = SafelyCastNetlinkMsgData<const struct ifinfomsg>(header, length);
                    if (msg && !IgnoreWirelessChange(msg, length))
                    {
                        auto id    = GetAdapterId(msg, IFLA_PAYLOAD(header));
                        auto state = CastState(msg);
                        if (auto it = impl.cache.find(id); it == impl.cache.end() || it->second != state)
                        {
                            impl.cache[id] = state;

                            impl.callbacks.Call(id, state);
                        }
                        break;
                    }
                }
                break;
                case RTM_DELLINK:
                    const struct ifinfomsg* msg = SafelyCastNetlinkMsgData<const struct ifinfomsg>(header, length);
                    if (msg)
                    {
                        auto id    = GetAdapterId(msg, IFLA_PAYLOAD(header));
                        auto state = NetAdapterNotify::ConnectState::kDisconnected;
                        if (auto it = impl.cache.find(id); it != impl.cache.end() && it->second != state)
                        {
                            impl.callbacks.Call(id, state);
                        }
                        impl.cache[id] = state;
                        break;
                    }
                }
            }
        }
    }
}

int GetAdapterFlags(const std::string& name)
{
    LinuxFileDescriptor netSocket(socket(AF_INET, SOCK_DGRAM, 0));
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

std::map<std::string, NetAdapterNotify::ConnectState> GetNetAdapterStates()
{
    namespace fs = std::filesystem;
    std::map<std::string, NetAdapterNotify::ConnectState> result;
    std::error_code                                       ec;
    for (const auto& item : fs::directory_iterator("/sys/class/net", ec))
    {
        if (!item.is_directory(ec))
        {
            continue;
        }
        auto name  = item.path().filename().string();
        auto flags = GetAdapterFlags(name);
        auto state =
            ((flags & IFF_RUNNING) && (flags & IFF_UP)) ? NetAdapterNotify::ConnectState::kConnected : NetAdapterNotify::ConnectState::kDisconnected;
        result.emplace(name, state);
    }
    return result;
}
} // namespace

NetAdapterNotify::NetAdapterNotify() : _impl(std::make_unique<NetAdapterNotifyImpl>())
{
}

NetAdapterNotify::NetAdapterNotify(NetAdapterNotify&& other) noexcept : _impl(std::move(other._impl))

{
}

NetAdapterNotify& NetAdapterNotify::operator=(NetAdapterNotify&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

NetAdapterNotify::~NetAdapterNotify()
{
    if (_impl)
    {
        assert(!_impl->run);
        assert(0 == _impl->notifyId);
    }
}

bool NetAdapterNotify::Start()
{
    std::lock_guard lock(_impl->mutex);
    if (_impl->run)
    {
        return false;
    }
    do
    {
        _impl->fileDescription = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        if (_impl->fileDescription.Empty())
        {
            break;
        }
        sockaddr_nl addr = {};
        addr.nl_family   = AF_NETLINK;
        addr.nl_groups   = RTMGRP_LINK;
        if (bind(_impl->fileDescription.Fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            break;
        }
        _impl->cache = GetNetAdapterStates();
        _impl->notify.Start();
        assert(0 == _impl->notifyId);
        _impl->notifyId = _impl->notify.AddFileDescriptorReadableStateCallback(
            _impl->fileDescription.FileDescriptor(), std::bind(&NetlinkChangeNotify, std::ref(*_impl))
        );
        _impl->run = true;
        return true;
    }
    while (false);
    _impl->fileDescription.Close();
    return false;
}
void NetAdapterNotify::Stop()
{
    std::lock_guard lock(_impl->mutex);
    if (_impl->run)
    {
        _impl->notify.Stop();
        if (_impl->notifyId)
        {
            _impl->notify.RemoveFileDescriptorStateCallback(_impl->notifyId);
            _impl->notifyId = 0;
        }
        _impl->fileDescription.Close();
        _impl->run = false;
    }
}
size_t NetAdapterNotify::AddConnectChangeCallback(
    const std::function<void(const std::string& adapterId, NetAdapterNotify::ConnectState state)>& callback
)
{
    return _impl->callbacks.AddCallback(callback);
}
bool NetAdapterNotify::RemoveConnectChangeCallback(size_t callbackId, bool wait)
{
    return _impl->callbacks.RemoveCallback(callbackId, wait);
}
} // namespace zeus
#endif