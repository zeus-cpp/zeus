#include "zeus/foundation/system/net_adapter.h"
#include <sstream>
#include <fmt/format.h>
#include "impl/net_adapter_impl.h"

namespace zeus
{
NetAdapter::NetAdapter() : _impl(std::make_unique<NetAdapterImpl>())
{
}
NetAdapter::~NetAdapter()
{
}

NetAdapter::NetAdapter(const NetAdapter &other) : _impl(std::make_unique<NetAdapterImpl>(*other._impl))
{
}

NetAdapter::NetAdapter(NetAdapter &&other) noexcept : _impl(std::move(other._impl))
{
}

NetAdapter &NetAdapter::operator=(const NetAdapter &other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

NetAdapter &NetAdapter::operator=(NetAdapter &&other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

std::string NetAdapter::GetId() const
{
    return _impl->id;
}
std::string NetAdapter::GetDisplayName() const
{
    return _impl->displayName;
}
std::string NetAdapter::GetDeviceName() const
{
    return _impl->deviceName;
}
std::string NetAdapter::GetMac(const std::string &split) const
{
    if (!_impl->physicalAddress.empty())
    {
        const auto        format      = "{:02X}";
        const auto        splitFormat = split + format;
        std::stringstream stream;
        stream.imbue(std::locale::classic());
        for (size_t index = 0; index < _impl->physicalAddress.size(); ++index)
        {
            if (0 == index)
            {
                stream << fmt::format(format, _impl->physicalAddress[index]);
            }
            else
            {
                stream << fmt::format(splitFormat, _impl->physicalAddress[index]);
            }
        }
        return stream.str();
    }
    return std::string();
}
std::vector<IPV4Address> NetAdapter::GetIPV4Address() const
{
    return _impl->v4AddressList;
}
std::vector<IPV6Address> NetAdapter::GetIPV6Address() const
{
    return _impl->v6AddressList;
}
std::vector<std::string> NetAdapter::GetIPV4Gateway() const
{
    return _impl->v4GatewayList;
}
std::vector<std::string> NetAdapter::GetIPV6Gateway() const
{
    return _impl->v6GatewayList;
}
uint64_t NetAdapter::GetSpeed() const
{
    return _impl->speed;
}
NetAdapter::NetAdapterType NetAdapter::GetType() const
{
    return _impl->type;
}
bool NetAdapter::IsConnected() const
{
    return _impl->isConnected;
}
bool NetAdapter::IsVirtual() const
{
    return _impl->isVirtual;
}
#ifdef _WIN32
uint32_t NetAdapter::GetV4Index() const
{
    return _impl->v4Index;
}

uint32_t NetAdapter::GetV6Index() const
{
    return _impl->v6Index;
}
bool NetAdapter::IsEnable() const
{
    return _impl->isEnable;
}
#endif
std::string NetAdapter::TypeName(NetAdapterType type)
{
    switch (type)
    {
    case NetAdapterType::Ethernet:
        return "Ethernet";
    case NetAdapterType::Wireless:
        return "Wireless";
    case NetAdapterType::Loopback:
        return "Loopback";
    default:
        return "Other";
    }
}

} // namespace zeus
