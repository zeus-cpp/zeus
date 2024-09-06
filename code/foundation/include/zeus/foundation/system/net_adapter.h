#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <optional>
#include "zeus/foundation/system/ip_address.h"

namespace zeus
{
struct NetAdapterImpl;
class NetAdapter
{
public:
    enum class NetAdapterType
    {
        Other,
        Ethernet,
        Wireless,
        Loopback,
    };

    enum class NetVersion
    {
        V4,
        V6,
    };
    NetAdapter();
    ~NetAdapter();
    NetAdapter(const NetAdapter& other);
    NetAdapter(NetAdapter&& other) noexcept;
    NetAdapter&              operator=(const NetAdapter& other);
    NetAdapter&              operator=(NetAdapter&& other) noexcept;
    std::string              GetId() const;
    std::string              GetDisplayName() const;
    std::string              GetDeviceName() const;
    std::string              GetMac(const std::string& split = ":") const;
    std::vector<IPV4Address> GetIPV4Address() const;
    std::vector<IPV6Address> GetIPV6Address() const;
    std::vector<std::string> GetIPV4Gateway() const;
    std::vector<std::string> GetIPV6Gateway() const;
    uint64_t                 GetSpeed() const;
    NetAdapterType           GetType() const;
    bool                     IsConnected() const;
    bool                     IsVirtual() const;
#ifdef _WIN32
    uint32_t GetV4Index() const;
    uint32_t GetV6Index() const;
    bool     IsEnable() const;
#endif
#ifdef __linux__
    uint32_t GetIndex() const;
#endif
public:
    static std::vector<NetAdapter>   List(NetAdapterType type, NetVersion version);
    static std::vector<NetAdapter>   ListAll();
    static std::optional<NetAdapter> FindNetAdapterById(const std::string& id);
    static std::string               TypeName(NetAdapterType type);
private:
    std::unique_ptr<NetAdapterImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
