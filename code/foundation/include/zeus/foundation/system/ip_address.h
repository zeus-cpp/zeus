#pragma once
#include <string>
#include <memory>

#ifdef _WIN32
#ifndef _WINSOCKAPI_
struct in_addr;
struct in6_addr;
#endif
#endif
#ifdef __linux__
#include <netinet/in.h>
#endif
namespace zeus
{
struct IPV4Addressmpl;
class IPV4Address
{
public:
    IPV4Address();
    ~IPV4Address();
    IPV4Address(const IPV4Address& other);
    IPV4Address(IPV4Address&& other) noexcept;
    IPV4Address& operator=(const IPV4Address& other);
    IPV4Address& operator=(IPV4Address&& other) noexcept;
    std::string  Address() const;
    std::string  Mask() const;
    size_t       PrefixLength() const;
public:
    static IPV4Address Parse(const in_addr* addr, size_t prefixLength);
    static IPV4Address Parse(const in_addr* addr, const in_addr* mask);
    static std::string ToString(const in_addr* addr);

private:
    std::unique_ptr<IPV4Addressmpl> _impl;
};

struct IPV6Addressmpl;
class IPV6Address
{
public:
    IPV6Address();
    ~IPV6Address();
    IPV6Address(const IPV6Address& other);
    IPV6Address(IPV6Address&& other) noexcept;
    IPV6Address& operator=(const IPV6Address& other);
    IPV6Address& operator=(IPV6Address&& other) noexcept;
    std::string  Address() const;
    size_t       PrefixLength() const;
public:
    static IPV6Address Parse(const in6_addr* addr, size_t prefixLength);
    static IPV6Address Parse(const in6_addr* addr, const in6_addr* mask);
    static std::string ToString(const in6_addr* addr);
private:
    std::unique_ptr<IPV6Addressmpl> _impl;
};

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
