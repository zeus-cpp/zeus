#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include "zeus/foundation/system/ip_address.h"
#include "zeus/foundation/system/net_adapter.h"

namespace zeus
{
struct NetAdapterNotifyImpl;
//windows下此类内部使用INetworkListManager的COM接口，此接口依赖windows的netprofm服务，此服务并不稳定，我们经常接到服务无法启动和网络状态不正确的报告，你并不应该在关键业务代码中依赖此类的正确运行，此类只能作为一种辅助手段或者在非关键业务中使用
//linux此类内部使用netlink套接字来监听网络状态变化，此类在桌面版linux下通常是可靠的，但是在某些定制版上需要注意内核是否启用了网络适配器相关的netlink消息，如果没有启用此类将无法工作
class NetAdapterNotify
{
public:
    enum class ConnectState
    {
        kDisconnected,
        kConnected,
#ifdef _WIN32
        kConnectedInternet,
#endif
    };
    NetAdapterNotify();
    ~NetAdapterNotify();
    NetAdapterNotify(const NetAdapterNotify&)            = delete;
    NetAdapterNotify& operator=(const NetAdapterNotify&) = delete;
    NetAdapterNotify(NetAdapterNotify&& other) noexcept;
    NetAdapterNotify& operator=(NetAdapterNotify&& other) noexcept;
    bool              Start();
    void              Stop();
    size_t AddConnectChangeCallback(const std::function<void(const std::string& adapterId, NetAdapterNotify::ConnectState state)>& callback);
    bool   RemoveConnectChangeCallback(size_t callbackId, bool wait = true);
public:
    static std::string StateName(ConnectState state);
private:
    std::unique_ptr<NetAdapterNotifyImpl> _impl;
};

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
