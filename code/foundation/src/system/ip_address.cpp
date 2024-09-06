#include "zeus/foundation/system/ip_address.h"
#include <cassert>
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif
#ifdef __linux__
#include <arpa/inet.h>
#endif

#include "zeus/foundation/byte/byte_utils.h"
#include "zeus/foundation/byte/byte_order.h"

namespace zeus
{
struct IPV4Addressmpl
{
    std::string address;
    std::string mask;
    size_t      prefixLength = 0;
};
IPV4Address::IPV4Address() : _impl(std::make_unique<IPV4Addressmpl>())
{
}
IPV4Address::~IPV4Address()
{
}
IPV4Address::IPV4Address(const IPV4Address& other) : _impl(std::make_unique<IPV4Addressmpl>(*other._impl))
{
}
IPV4Address::IPV4Address(IPV4Address&& other) noexcept : _impl(std::move(other._impl))
{
}
IPV4Address& IPV4Address::operator=(const IPV4Address& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
IPV4Address& IPV4Address::operator=(IPV4Address&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string IPV4Address::Address() const
{
    return _impl->address;
}
std::string IPV4Address::Mask() const
{
    return _impl->mask;
}
size_t IPV4Address::PrefixLength() const
{
    return _impl->prefixLength;
}

IPV4Address IPV4Address::Parse(const in_addr* addr, size_t prefixLength)
{
    assert(addr);
    assert(prefixLength <= 32);
    IPV4Address address;
    address._impl->address      = ToString(addr);
    address._impl->prefixLength = prefixLength;
    const uint32_t prefix       = (int32_t {1 << 31}) >> (prefixLength - 1);
    in_addr        mask         = {};
#ifdef _WIN32
    mask.S_un.S_addr = FlipBytes(&prefix);
#endif
#ifdef __linux__
    mask.s_addr = FlipBytes(&prefix);
#endif
    address._impl->mask = ToString(&mask);
    return address;
}
IPV4Address IPV4Address::Parse(const in_addr* addr, const in_addr* mask)
{
    assert(addr);
    assert(mask);
    IPV4Address address;
    address._impl->address      = ToString(addr);
    address._impl->mask         = ToString(mask);
    address._impl->prefixLength = CountOne({reinterpret_cast<const uint8_t*>(mask), sizeof(in_addr)});
    return address;
}
std::string IPV4Address::ToString(const in_addr* addr)
{
    char buffer[INET_ADDRSTRLEN] = {0};
    return inet_ntop(AF_INET, addr, buffer, INET_ADDRSTRLEN);
}

struct IPV6Addressmpl
{
    std::string address;
    size_t      prefixLength = 0;
};
IPV6Address::IPV6Address() : _impl(std::make_unique<IPV6Addressmpl>())
{
}
IPV6Address::~IPV6Address()
{
}
IPV6Address::IPV6Address(const IPV6Address& other) : _impl(std::make_unique<IPV6Addressmpl>(*other._impl))
{
}
IPV6Address::IPV6Address(IPV6Address&& other) noexcept : _impl(std::move(other._impl))
{
}
IPV6Address& IPV6Address::operator=(const IPV6Address& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
IPV6Address& IPV6Address::operator=(IPV6Address&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string IPV6Address::Address() const
{
    return _impl->address;
}
size_t IPV6Address::PrefixLength() const
{
    return _impl->prefixLength;
}
IPV6Address IPV6Address::Parse(const in6_addr* addr, size_t prefixLength)
{
    assert(addr);
    assert(prefixLength <= 128);
    IPV6Address address;
    address._impl->address      = ToString(addr);
    address._impl->prefixLength = prefixLength;
    return address;
}
IPV6Address IPV6Address::Parse(const in6_addr* addr, const in6_addr* mask)
{
    assert(addr);
    assert(mask);
    IPV6Address address;
    address._impl->address      = ToString(addr);
    auto maskString             = ToString(mask);
    address._impl->prefixLength = CountOne({reinterpret_cast<const uint8_t*>(mask), sizeof(in6_addr)});
    return address;
}
std::string IPV6Address::ToString(const in6_addr* addr)
{
    char buffer[INET6_ADDRSTRLEN] = {0};
    return inet_ntop(AF_INET6, addr, buffer, INET6_ADDRSTRLEN);
}

} // namespace zeus