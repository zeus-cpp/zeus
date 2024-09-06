#include "zeus/foundation/system/win/firewall_policy_manager.h"
#ifdef _WIN32
#include <cassert>
#include <atlbase.h>
#include <netfw.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/core/win/com_error.h"
#include "zeus/foundation/crypt/uuid.h"
#include "zeus/foundation/resource/win/com_init.h"
#include "zeus/foundation/thread/thread_checker.h"
#include "zeus/foundation/string/charset_utils.h"
namespace zeus
{

namespace
{
NET_FW_RULE_DIRECTION DirectionCast(WinFirewallRule::Direction direction)
{
    switch (direction)
    {
    case WinFirewallRule::Direction::kInbound:
        return NET_FW_RULE_DIR_IN;
    case WinFirewallRule::Direction::kOutbound:
        return NET_FW_RULE_DIR_OUT;
    default:
        assert(false);
        return NET_FW_RULE_DIR_IN;
    }
}

WinFirewallRule::Direction DirectionCast(NET_FW_RULE_DIRECTION direction)
{
    switch (direction)
    {
    case NET_FW_RULE_DIR_IN:
        return WinFirewallRule::Direction::kInbound;
    case NET_FW_RULE_DIR_OUT:
        return WinFirewallRule::Direction::kOutbound;
    default:
        assert(false);
        return WinFirewallRule::Direction::kInbound;
    }
}

NET_FW_ACTION_ ActionCast(WinFirewallRule::Action action)
{
    switch (action)
    {
    case WinFirewallRule::Action::kAllow:
        return NET_FW_ACTION_ALLOW;
    case WinFirewallRule::Action::kBlock:
        return NET_FW_ACTION_BLOCK;
    default:
        assert(false);
        return NET_FW_ACTION_ALLOW;
    }
}

WinFirewallRule::Action ActionCast(NET_FW_ACTION_ action)
{
    switch (action)
    {
    case NET_FW_ACTION_ALLOW:
        return WinFirewallRule::Action::kAllow;
    case NET_FW_ACTION_BLOCK:
        return WinFirewallRule::Action::kBlock;
    default:
        assert(false);
        return WinFirewallRule::Action::kAllow;
    }
}

NET_FW_PROFILE_TYPE2 ProfileCast(WinFirewallRule::Profile profile)
{
    using Type                = std::underlying_type_t<WinFirewallRule::Profile>;
    const bool domainProfile  = static_cast<Type>(WinFirewallRule::Profile::kDomain & profile);
    const bool privateProfile = static_cast<Type>(WinFirewallRule::Profile::kPrivate & profile);
    const bool publicProfile  = static_cast<Type>(WinFirewallRule::Profile::kPublic & profile);
    if (domainProfile && privateProfile && publicProfile)
    {
        return NET_FW_PROFILE2_ALL;
    }
    long type = {};
    if (domainProfile)
    {
        type |= NET_FW_PROFILE2_DOMAIN;
    }
    if (privateProfile)
    {
        type |= NET_FW_PROFILE2_PRIVATE;
    }
    if (publicProfile)
    {
        type |= NET_FW_PROFILE2_PUBLIC;
    }
    return static_cast<NET_FW_PROFILE_TYPE2>(type);
}

WinFirewallRule::Profile ProfileCast(long profile)
{
    if (NET_FW_PROFILE2_ALL == profile)
    {
        return WinFirewallRule::Profile::kDomain | WinFirewallRule::Profile::kPrivate | WinFirewallRule::Profile::kPublic;
    }
    const bool               domainProfile  = NET_FW_PROFILE2_DOMAIN & profile;
    const bool               privateProfile = NET_FW_PROFILE2_PRIVATE & profile;
    const bool               publicProfile  = NET_FW_PROFILE2_PUBLIC & profile;
    WinFirewallRule::Profile type           = {};
    if (domainProfile)
    {
        type |= WinFirewallRule::Profile::kDomain;
    }
    if (privateProfile)
    {
        type |= WinFirewallRule::Profile::kPrivate;
    }
    if (publicProfile)
    {
        type |= WinFirewallRule::Profile::kPublic;
    }
    return type;
}
} // namespace

struct WinFirewallRuleImpl
{
    CComPtr<INetFwRule> rule;
};
WinFirewallRule::WinFirewallRule() : _impl(std::make_unique<WinFirewallRuleImpl>())
{
}
WinFirewallRule::WinFirewallRule(WinFirewallRule&& other) noexcept : _impl(std::move(other._impl))
{
}
WinFirewallRule& WinFirewallRule::operator=(WinFirewallRule&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    _impl = std::move(other._impl);
    return *this;
}
WinFirewallRule::~WinFirewallRule()
{
}

zeus::expected<void, std::error_code> WinFirewallRule::SetName(const std::string& name)
{
    const HRESULT hr = _impl->rule->put_Name(CComBSTR(CharsetUtils::UTF8ToUnicode(name).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetName()
{
    CComBSTR      name;
    const HRESULT hr = _impl->rule->get_Name(&name);
    if (SUCCEEDED(hr) && name)
    {
        return CharsetUtils::UnicodeToUTF8(name.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetDescription(const std::string& description)
{
    const HRESULT hr = _impl->rule->put_Description(CComBSTR(CharsetUtils::UTF8ToUnicode(description).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetDescription()
{
    CComBSTR      description;
    const HRESULT hr = _impl->rule->get_Description(&description);
    if (SUCCEEDED(hr) && description)
    {
        return CharsetUtils::UnicodeToUTF8(description.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetApplicationName(const std::string& name)
{
    const HRESULT hr = _impl->rule->put_ApplicationName(CComBSTR(CharsetUtils::UTF8ToUnicode(name).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetApplicationName()
{
    CComBSTR      name;
    const HRESULT hr = _impl->rule->get_ApplicationName(&name);
    if (SUCCEEDED(hr) && name)
    {
        return CharsetUtils::UnicodeToUTF8(name.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetProtocol(Protocol protocol)
{
    const HRESULT hr = _impl->rule->put_Protocol(static_cast<NET_FW_IP_PROTOCOL>(protocol));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<WinFirewallRule::Protocol, std::error_code> WinFirewallRule::GetProtocol()
{
    LONG          protocol = {};
    const HRESULT hr       = _impl->rule->get_Protocol(&protocol);
    if (SUCCEEDED(hr))
    {
        return static_cast<Protocol>(protocol);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetLocalPorts(const std::string& ports)
{
    const HRESULT hr = _impl->rule->put_LocalPorts(CComBSTR(CharsetUtils::UTF8ToUnicode(ports).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetLocalPorts()
{
    CComBSTR      ports;
    const HRESULT hr = _impl->rule->get_LocalPorts(&ports);
    if (SUCCEEDED(hr) && ports)
    {
        return CharsetUtils::UnicodeToUTF8(ports.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetRemotePorts(const std::string& ports)
{
    const HRESULT hr = _impl->rule->put_RemotePorts(CComBSTR(CharsetUtils::UTF8ToUnicode(ports).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetRemotePorts()
{
    CComBSTR      ports;
    const HRESULT hr = _impl->rule->get_RemotePorts(&ports);
    if (SUCCEEDED(hr) && ports)
    {
        return CharsetUtils::UnicodeToUTF8(ports.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetLocalAddresses(const std::string& addresses)
{
    const HRESULT hr = _impl->rule->put_LocalAddresses(CComBSTR(CharsetUtils::UTF8ToUnicode(addresses).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetLocalAddresses()
{
    CComBSTR      addresses;
    const HRESULT hr = _impl->rule->get_LocalAddresses(&addresses);
    if (SUCCEEDED(hr) && addresses)
    {
        return CharsetUtils::UnicodeToUTF8(addresses.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetRemoteAddresses(const std::string& addresses)
{
    const HRESULT hr = _impl->rule->put_RemoteAddresses(CComBSTR(CharsetUtils::UTF8ToUnicode(addresses).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetRemoteAddresses()
{
    CComBSTR      addresses;
    const HRESULT hr = _impl->rule->get_RemoteAddresses(&addresses);
    if (SUCCEEDED(hr) && addresses)
    {
        return CharsetUtils::UnicodeToUTF8(addresses.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetDirection(Direction direction)
{
    const HRESULT hr = _impl->rule->put_Direction(DirectionCast(direction));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<WinFirewallRule::Direction, std::error_code> WinFirewallRule::GetDirection()
{
    NET_FW_RULE_DIRECTION_ direction {};
    const HRESULT          hr = _impl->rule->get_Direction(&direction);
    if (SUCCEEDED(hr))
    {
        return DirectionCast(direction);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetAction(Action action)
{
    const HRESULT hr = _impl->rule->put_Action(ActionCast(action));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<WinFirewallRule::Action, std::error_code> WinFirewallRule::GetAction()
{
    NET_FW_ACTION_ action {};
    const HRESULT  hr = _impl->rule->get_Action(&action);
    if (SUCCEEDED(hr))
    {
        return ActionCast(action);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetProfile(Profile profile)
{
    const HRESULT hr = _impl->rule->put_Profiles(ProfileCast(profile));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<WinFirewallRule::Profile, std::error_code> WinFirewallRule::GetProfile()
{
    long          profile {};
    const HRESULT hr = _impl->rule->get_Profiles(&profile);
    if (SUCCEEDED(hr))
    {
        return ProfileCast(profile);
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetEnabled(bool enabled)
{
    const HRESULT hr = _impl->rule->put_Enabled(enabled ? VARIANT_TRUE : VARIANT_FALSE);
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<bool, std::error_code> WinFirewallRule::GetEnabled()
{
    VARIANT_BOOL  enabled {};
    const HRESULT hr = _impl->rule->get_Enabled(&enabled);
    if (SUCCEEDED(hr))
    {
        return enabled == VARIANT_TRUE;
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallRule::SetGrouping(const std::string& grouping)
{
    const HRESULT hr = _impl->rule->put_Grouping(CComBSTR(CharsetUtils::UTF8ToUnicode(grouping).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<std::string, std::error_code> WinFirewallRule::GetGrouping()
{
    CComBSTR      grouping;
    const HRESULT hr = _impl->rule->get_Grouping(&grouping);
    if (SUCCEEDED(hr) && grouping)
    {
        return CharsetUtils::UnicodeToUTF8(grouping.operator LPWSTR());
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

struct WinFirewallPolicyManagerImpl
{
    ComInit                comInit;
    CComPtr<INetFwPolicy2> firewallPolicy;
    CComPtr<INetFwRules>   rules;
    ThreadChecker          checker;
};
WinFirewallPolicyManager::WinFirewallPolicyManager() : _impl(std::make_unique<WinFirewallPolicyManagerImpl>())
{
}
WinFirewallPolicyManager::WinFirewallPolicyManager(WinFirewallPolicyManager&& other) noexcept : _impl(std::move(other._impl))
{
}
WinFirewallPolicyManager& WinFirewallPolicyManager::operator=(WinFirewallPolicyManager&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    _impl = std::move(other._impl);
    return *this;
}
WinFirewallPolicyManager::~WinFirewallPolicyManager()
{
}
zeus::expected<WinFirewallRule, std::error_code> WinFirewallPolicyManager::CreateRule()
{
    CComPtr<INetFwRule> rule;
    const HRESULT hr = CoCreateInstance(__uuidof(NetFwRule), nullptr, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), reinterpret_cast<LPVOID*>(&rule));
    if (SUCCEEDED(hr))
    {
        WinFirewallRule firewallRule;
        firewallRule._impl->rule = rule;
        return firewallRule;
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<std::vector<WinFirewallRule>, std::error_code> WinFirewallPolicyManager::ListAllRule()
{
    long    count = 0;
    HRESULT hr    = _impl->rules->get_Count(&count);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    CComPtr<IUnknown> enumerator;
    hr = _impl->rules->get__NewEnum(&enumerator);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    CComPtr<IEnumVARIANT> enumVariant;
    hr = enumerator->QueryInterface(__uuidof(IEnumVARIANT), reinterpret_cast<void**>(&enumVariant));
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    std::vector<WinFirewallRule> result;
    result.reserve(count);
    ULONG cFetched = 0;
    while (true)
    {
        CComVariant variant;
        hr = enumVariant->Next(1, &variant, &cFetched);
        if (SUCCEEDED(hr) && hr != S_FALSE)
        {
            CComPtr<INetFwRule> rule;
            hr = V_DISPATCH(&variant)->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&rule));
            if (SUCCEEDED(hr))
            {
                WinFirewallRule firewallRule;
                firewallRule._impl->rule = rule;
                result.emplace_back(std::move(firewallRule));
            }
        }
        else
        {
            break;
        }
    }
    return result;
}

zeus::expected<WinFirewallRule, std::error_code> WinFirewallPolicyManager::GetRule(const std::string& name)
{
    CComPtr<INetFwRule> rule;
    const HRESULT       hr = _impl->rules->Item(CComBSTR(CharsetUtils::UTF8ToUnicode(name).c_str()), &rule);
    if (SUCCEEDED(hr))
    {
        WinFirewallRule firewallRule;
        firewallRule._impl->rule = rule;
        return firewallRule;
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}

zeus::expected<void, std::error_code> WinFirewallPolicyManager::AddRule(const WinFirewallRule& rule)
{
    const HRESULT hr = _impl->rules->Add(rule._impl->rule);
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<void, std::error_code> WinFirewallPolicyManager::RemoveRule(WinFirewallRule& rule)
{
    auto name = Uuid::GenerateRandom().toWindowsString();
    rule.SetName(name);
    return RemoveRule(name);
}
zeus::expected<void, std::error_code> WinFirewallPolicyManager::RemoveRule(const std::string& name)
{
    const HRESULT hr = _impl->rules->Remove(CComBSTR(CharsetUtils::UTF8ToUnicode(name).c_str()));
    if (SUCCEEDED(hr))
    {
        return {};
    }
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
zeus::expected<WinFirewallPolicyManager, std::error_code> WinFirewallPolicyManager::Create()
{
    ComInit                comInit;
    CComPtr<INetFwPolicy2> firewallPolicy;
    CComPtr<INetFwRules>   rules;
    HRESULT                hr =
        CoCreateInstance(__uuidof(NetFwPolicy2), nullptr, CLSCTX_INPROC_SERVER, __uuidof(INetFwPolicy2), reinterpret_cast<LPVOID*>(&firewallPolicy));
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    hr = firewallPolicy->get_Rules(&rules);
    if (FAILED(hr))
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
    WinFirewallPolicyManager manager;
    manager._impl->comInit        = std::move(comInit);
    manager._impl->firewallPolicy = std::move(firewallPolicy);
    manager._impl->rules          = std::move(rules);
    return manager;
}

} // namespace zeus
#endif