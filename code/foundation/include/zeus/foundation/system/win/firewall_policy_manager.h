#pragma once
#ifdef _WIN32
#include <memory>
#include <string>
#include <system_error>
#include <zeus/expected.hpp>
#include "zeus/foundation/core/enum_bit_operator.h"

namespace zeus
{
struct WinFirewallRuleImpl;
class WinFirewallRule
{
public:
    enum class Direction
    {
        kInbound,
        kOutbound,
    };
    enum class Action
    {
        kAllow,
        kBlock,
    };

    enum class Profile
    {
        kDomain  = 0x1,
        kPrivate = 0x2,
        kPublic  = 0x4,
    }; // 定义位操作符重载函数

    enum class Protocol
    {
        kTcp = 6,
        kUdp = 17,
        kAny = 256,
        //protocol value https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xml
    };

    WinFirewallRule(const WinFirewallRule&) = delete;
    WinFirewallRule(WinFirewallRule&& other) noexcept;
    WinFirewallRule& operator=(const WinFirewallRule&) = delete;
    WinFirewallRule& operator=(WinFirewallRule&& other) noexcept;
    ~WinFirewallRule();

    zeus::expected<void, std::error_code>        SetName(const std::string& name);
    zeus::expected<std::string, std::error_code> GetName();
    zeus::expected<void, std::error_code>        SetDescription(const std::string& description);
    zeus::expected<std::string, std::error_code> GetDescription();
    zeus::expected<void, std::error_code>        SetApplicationName(const std::string& name);
    zeus::expected<std::string, std::error_code> GetApplicationName();
    zeus::expected<void, std::error_code>        SetProtocol(Protocol protocol);
    zeus::expected<Protocol, std::error_code>    GetProtocol();
    zeus::expected<void, std::error_code>        SetLocalPorts(const std::string& ports);
    zeus::expected<std::string, std::error_code> GetLocalPorts();
    zeus::expected<void, std::error_code>        SetRemotePorts(const std::string& ports);
    zeus::expected<std::string, std::error_code> GetRemotePorts();
    zeus::expected<void, std::error_code>        SetLocalAddresses(const std::string& addresses);
    zeus::expected<std::string, std::error_code> GetLocalAddresses();
    zeus::expected<void, std::error_code>        SetRemoteAddresses(const std::string& addresses);
    zeus::expected<std::string, std::error_code> GetRemoteAddresses();
    zeus::expected<void, std::error_code>        SetDirection(Direction direction);
    zeus::expected<Direction, std::error_code>   GetDirection();
    zeus::expected<void, std::error_code>        SetAction(Action action);
    zeus::expected<Action, std::error_code>      GetAction();
    zeus::expected<void, std::error_code>        SetProfile(Profile profile);
    zeus::expected<Profile, std::error_code>     GetProfile();
    zeus::expected<void, std::error_code>        SetEnabled(bool enabled);
    zeus::expected<bool, std::error_code>        GetEnabled();
    zeus::expected<void, std::error_code>        SetGrouping(const std::string& grouping);
    zeus::expected<std::string, std::error_code> GetGrouping();
protected:
    WinFirewallRule();
private:
    std::unique_ptr<WinFirewallRuleImpl> _impl;
    friend class WinFirewallPolicyManager;
};

ZEUS_ENUM_BIT_OPERATOR(WinFirewallRule::Profile)

struct WinFirewallPolicyManagerImpl;
class WinFirewallPolicyManager
{
public:
    WinFirewallPolicyManager(const WinFirewallPolicyManager&) = delete;
    WinFirewallPolicyManager(WinFirewallPolicyManager&& other) noexcept;
    WinFirewallPolicyManager& operator=(const WinFirewallPolicyManager&) = delete;
    WinFirewallPolicyManager& operator=(WinFirewallPolicyManager&& other) noexcept;
    ~WinFirewallPolicyManager();

    zeus::expected<WinFirewallRule, std::error_code>              CreateRule();
    zeus::expected<std::vector<WinFirewallRule>, std::error_code> ListAllRule();
    zeus::expected<WinFirewallRule, std::error_code>              GetRule(const std::string& name);
    zeus::expected<void, std::error_code>                         AddRule(const WinFirewallRule& rule);
    zeus::expected<void, std::error_code>                         RemoveRule(WinFirewallRule& rule);
    zeus::expected<void, std::error_code>                         RemoveRule(const std::string& name);
public:
    static zeus::expected<WinFirewallPolicyManager, std::error_code> Create();
protected:
    WinFirewallPolicyManager();
private:
    std::unique_ptr<WinFirewallPolicyManagerImpl> _impl;
};
} // namespace zeus
// namespace zeus
#endif