#pragma once

#ifdef _WIN32
#include <vector>
#include <memory>
#include <system_error>
#include <utility>
#include <zeus/expected.hpp>
#include <zeus/foundation/core/win/win_windef.h>
#include <zeus/foundation/resource/win/handle.h>
#include <zeus/foundation/system/win/session.h>
#include <zeus/foundation/system/win/sid.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/security/win/privilege.h>

namespace zeus
{
struct WinTokenImpl;
class WinToken
{
public:
    enum class WinTokenElevationType
    {
        kDefault,
        kFull,
        kLimited,
        kUnknown,
    };

    enum class WinTokenIntegrityLevel
    {
        kUntrusted,
        kLow,
        kMedium,
        kMediumPlus,
        kHigh,
        kSystem,
        Protected,
        kUnknown,
    };
    enum class SecurityImpersonationLevel
    {
        Anonymous = 0,
        Identification,
        Impersonation,
        Delegation,
    };
    WinToken();
    explicit WinToken(HANDLE token);
    WinToken(const WinToken &other) = delete;
    WinToken(WinToken &&other) noexcept;
    WinToken(WinHandle &&token) noexcept;
    WinToken &operator=(const WinToken &other) = delete;
    WinToken &operator=(WinToken &&other) noexcept;
    WinToken &operator=(WinHandle &&other) noexcept;
    ~WinToken();
    HANDLE *operator&() noexcept;
    operator HANDLE() const noexcept;
    bool                                                    operator==(const WinToken &other) const noexcept;
    bool                                                    operator!=(const WinToken &other) const noexcept;
    bool                                                    operator==(const WinHandle &handle) const noexcept;
    bool                                                    operator!=(const WinHandle &handle) const noexcept;
    bool                                                    operator==(HANDLE handle) const noexcept;
    bool                                                    operator!=(HANDLE handle) const noexcept;
    HANDLE                                                  Handle() const noexcept;
    bool                                                    Empty() const noexcept;
    zeus::expected<bool, std::error_code>                   HasLinkToken() const;
    zeus::expected<bool, std::error_code>                   IsPrimary() const;
    zeus::expected<WinSession::SessionId, std::error_code>  GetSessionId() const;
    zeus::expected<WinToken, std::error_code>               GetLinkerToken() const;
    zeus::expected<bool, std::error_code>                   IsUiAccess() const;
    zeus::expected<WinTokenElevationType, std::error_code>  GetElevationType() const;
    zeus::expected<WinTokenIntegrityLevel, std::error_code> GetIntegrityLevel() const;
    zeus::expected<WinSID, std::error_code>                 GetUserSID() const;
    zeus::expected<WinAccount, std::error_code>             GetUserAccount() const;
    zeus::expected<std::string, std::error_code>            GetUserSIDString() const;
    zeus::expected<std::vector<uint8_t>, std::error_code>   GetInformation(int type) const;
    zeus::expected<void, std::error_code>                   SetInformation(int type, const void *data, uint32_t size);
    zeus::expected<void, std::error_code>                   SetSessionId(WinSession::SessionId sessionId);
    zeus::expected<void, std::error_code>                   SetUiAccess(bool enable);
    zeus::expected<void, std::error_code>                   EnablePrivileges(const WinPrivilege &privilege, bool enable);
    zeus::expected<void, std::error_code>                   EnablePrivileges(WinPrivilege::Privilege privilegeName, bool enable);
    zeus::expected<bool, std::error_code>                   HasPrivilege(const WinPrivilege::Privilege &privilegeName) const;
    zeus::expected<bool, std::error_code>                   HasPrivilege(const WinPrivilege &privilege) const;
    zeus::expected<WinToken, std::error_code>               Duplicate(bool primary, SecurityImpersonationLevel level) const;
    zeus::expected<bool, std::error_code>                   HasAdmin() const;
public:
    static WinToken                                  GetCurrentProcessToken();
    static WinToken                                  GetCurrentThreadToken();
    static zeus::expected<bool, std::error_code>     CurrentProcessHasAdmin();
    static zeus::expected<bool, std::error_code>     CurrentThreadHasAdmin();
    //此函数只在系统服务权限下可用
    static zeus::expected<WinToken, std::error_code> GetSessionToken(WinSession::SessionId session);
    static zeus::expected<WinToken, std::error_code> GetProcessToken(Process::PID pid);
    static zeus::expected<WinToken, std::error_code> GetProcessToken(const Process &process);
    static zeus::expected<WinToken, std::error_code> GetProcessToken(HANDLE processHandle);
private:
    std::unique_ptr<WinTokenImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
