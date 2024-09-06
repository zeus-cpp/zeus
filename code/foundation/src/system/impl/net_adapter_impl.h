#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <thread>
#include "zeus/foundation/system/ip_address.h"

namespace zeus
{
struct NetAdapterImpl
{
    std::string                id;
    std::string                displayName;
    std::string                deviceName;
    std::vector<IPV4Address>   v4AddressList;
    std::vector<IPV6Address>   v6AddressList;
    std::vector<std::string>   v4GatewayList;
    std::vector<std::string>   v6GatewayList;
    std::vector<uint8_t>       physicalAddress;
    NetAdapter::NetAdapterType type        = NetAdapter::NetAdapterType::Other;
    uint64_t                   speed       = 0;
    bool                       isVirtual   = false;
    bool                       isConnected = false;
#ifdef _WIN32
    uint32_t v4Index  = 0;
    uint32_t v6Index  = 0;
    bool     isEnable = false;
#endif
#ifdef __linux__
    uint32_t index = 0;
#endif
};
} // namespace zeus
