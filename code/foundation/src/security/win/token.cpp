#include "zeus/foundation/security/win/token.h"

#ifdef _WIN32
#include <Windows.h>
#include <cassert>
#include <WtsApi32.h>
#include "zeus/foundation/core/system_error.h"

#pragma comment(lib, "WtsApi32.lib")

namespace zeus
{
namespace
{
zeus::expected<std::vector<uint8_t>, std::error_code> GetTokenInfo(HANDLE token, TOKEN_INFORMATION_CLASS type)
{
    DWORD len = 0;
    GetTokenInformation(token, type, nullptr, 0, &len);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        std::vector<uint8_t> data(len);
        if (GetTokenInformation(token, type, data.data(), len, &len))
        {
            return std::move(data);
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(TranslateToSystemError(error));
    }
}
} // namespace
struct WinTokenImpl
{
    WinHandle handle;
};
WinToken::WinToken() : _impl(std::make_unique<WinTokenImpl>())
{
}

WinToken::WinToken(HANDLE token)
{
    _impl = std::make_unique<WinTokenImpl>();
    _impl->handle.Attach(token);
}

WinToken::WinToken(WinToken &&other) noexcept : _impl(std::move(other._impl))
{
}

WinToken::WinToken(WinHandle &&token) noexcept : _impl(std::make_unique<WinTokenImpl>())
{
    _impl->handle = std::move(token);
}

WinToken &WinToken::operator=(WinToken &&other) noexcept
{
    if (this != std::addressof(other))
    {
        _impl.swap(other._impl);
    }
    return *this;
}

WinToken &WinToken::operator=(WinHandle &&other) noexcept
{
    _impl->handle = std::move(other);
    return *this;
}

WinToken::~WinToken()
{
}

HANDLE *WinToken::operator&() noexcept
{
    assert(_impl->handle.Empty());
    return &_impl->handle;
}
WinToken::operator HANDLE() const noexcept
{
    return _impl->handle;
}
bool WinToken::operator==(const WinToken &other) const noexcept
{
    return _impl->handle == other._impl->handle;
}
bool WinToken::operator!=(const WinToken &other) const noexcept
{
    return _impl->handle != other._impl->handle;
}
bool WinToken::operator==(const WinHandle &handle) const noexcept
{
    return _impl->handle == handle.Handle();
}
bool WinToken::operator!=(const WinHandle &handle) const noexcept
{
    return _impl->handle != handle.Handle();
}
bool WinToken::operator==(HANDLE handle) const noexcept
{
    return _impl->handle == handle;
}
bool WinToken::operator!=(HANDLE handle) const noexcept
{
    return _impl->handle != handle;
}
HANDLE WinToken::Handle() const noexcept
{
    return _impl->handle;
}
bool WinToken::Empty() const noexcept
{
    return _impl->handle.Empty();
}

zeus::expected<bool, std::error_code> WinToken::HasLinkToken() const
{
    DWORD     size = 0;
    WinHandle linkedToken;
    if (GetTokenInformation(_impl->handle, TokenLinkedToken, &linkedToken, sizeof(HANDLE), &size))
    {
        return !linkedToken.Empty();
    }
    else if (ERROR_NO_SUCH_LOGON_SESSION == GetLastError())
    {
        return false;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<bool, std::error_code> WinToken::IsPrimary() const
{
    DWORD      size = 0;
    TOKEN_TYPE type = {};
    if (GetTokenInformation(_impl->handle, TokenType, &type, sizeof(TOKEN_TYPE), &size))
    {
        return type == TOKEN_TYPE::TokenPrimary;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<WinSession::SessionId, std::error_code> WinToken::GetSessionId() const
{
    DWORD                 size = 0;
    WinSession::SessionId id   = 0xFFFFFFFF;
    if (GetTokenInformation(_impl->handle, TokenSessionId, &id, sizeof(WinSession::SessionId), &size))
    {
        return id;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<WinToken, std::error_code> WinToken::GetLinkerToken() const
{
    DWORD  size        = 0;
    HANDLE linkedToken = nullptr;
    if (GetTokenInformation(_impl->handle, TokenLinkedToken, &linkedToken, sizeof(HANDLE), &size))
    {
        return WinToken(linkedToken);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<bool, std::error_code> WinToken::IsUiAccess() const
{
    DWORD size     = 0;
    DWORD uiAccess = 0;
    if (GetTokenInformation(_impl->handle, TokenUIAccess, &uiAccess, sizeof(DWORD), &size))
    {
        return uiAccess != 0;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<WinToken::WinTokenElevationType, std::error_code> WinToken::GetElevationType() const
{
    DWORD                size = 0;
    TOKEN_ELEVATION_TYPE type = {};
    if (GetTokenInformation(_impl->handle, TokenElevationType, &type, sizeof(TOKEN_ELEVATION_TYPE), &size))
    {
        switch (type)
        {
        case TOKEN_ELEVATION_TYPE::TokenElevationTypeFull:
            return WinTokenElevationType::kFull;
        case TOKEN_ELEVATION_TYPE::TokenElevationTypeLimited:
            return WinTokenElevationType::kLimited;
        case TOKEN_ELEVATION_TYPE::TokenElevationTypeDefault:
            return WinTokenElevationType::kDefault;
        default:
            return WinTokenElevationType::kUnknown;
        }
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<WinToken::WinTokenIntegrityLevel, std::error_code> WinToken::GetIntegrityLevel() const
{
    auto buffer = GetTokenInfo(_impl->handle, TokenIntegrityLevel);
    if (buffer.has_value())
    {
        SID *sid = reinterpret_cast<SID *>(reinterpret_cast<TOKEN_MANDATORY_LABEL *>(buffer->data())->Label.Sid);
        switch (sid->SubAuthority[0])
        {
        case SECURITY_MANDATORY_UNTRUSTED_RID:
            return WinTokenIntegrityLevel::kUntrusted;
        case SECURITY_MANDATORY_LOW_RID:
            return WinTokenIntegrityLevel::kLow;
        case SECURITY_MANDATORY_MEDIUM_RID:
            return WinTokenIntegrityLevel::kMedium;
        case SECURITY_MANDATORY_MEDIUM_PLUS_RID:
            return WinTokenIntegrityLevel::kMediumPlus;
        case SECURITY_MANDATORY_HIGH_RID:
            return WinTokenIntegrityLevel::kHigh;
        case SECURITY_MANDATORY_SYSTEM_RID:
            return WinTokenIntegrityLevel::kSystem;
        case SECURITY_MANDATORY_PROTECTED_PROCESS_RID:
            return WinTokenIntegrityLevel::Protected;
        default:
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(buffer.error());
    }
}
zeus::expected<WinSID, std::error_code> WinToken::GetUserSID() const
{
    auto buffer = GetTokenInfo(_impl->handle, TokenUser);

    if (buffer.has_value())
    {
        return WinSID::CopySID(reinterpret_cast<TOKEN_USER *>(buffer->data())->User.Sid);
    }
    else
    {
        return zeus::unexpected(buffer.error());
    }
}
zeus::expected<WinAccount, std::error_code> WinToken::GetUserAccount() const
{
    auto buffer = GetTokenInfo(_impl->handle, TokenUser);
    if (buffer.has_value())
    {
        return WinSID::GetSIDAccount(reinterpret_cast<TOKEN_USER *>(buffer->data())->User.Sid);
    }
    else
    {
        return zeus::unexpected(buffer.error());
    }
}
zeus::expected<std::string, std::error_code> WinToken::GetUserSIDString() const
{
    auto buffer = GetTokenInfo(_impl->handle, TokenUser);
    if (buffer.has_value())
    {
        return WinSID::GetSIDString(reinterpret_cast<TOKEN_USER *>(buffer->data())->User.Sid);
    }
    else
    {
        return zeus::unexpected(buffer.error());
    }
}
zeus::expected<std::vector<uint8_t>, std::error_code> WinToken::GetInformation(int type) const
{
    return GetTokenInfo(_impl->handle, static_cast<TOKEN_INFORMATION_CLASS>(type));
}
zeus::expected<void, std::error_code> WinToken::SetInformation(int type, const void *data, uint32_t size)
{
    if (SetTokenInformation(_impl->handle, static_cast<TOKEN_INFORMATION_CLASS>(type), const_cast<void *>(data), size))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<void, std::error_code> WinToken::SetSessionId(WinSession::SessionId sessionId)
{
    if (SetTokenInformation(_impl->handle, TokenSessionId, &sessionId, sizeof(sessionId)))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinToken::SetUiAccess(bool enable)
{
    DWORD access = enable ? 1 : 0;
    if (SetTokenInformation(_impl->handle, TokenUIAccess, &access, sizeof(access)))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinToken::EnablePrivileges(const WinPrivilege &privilege, bool enable)
{
    TOKEN_PRIVILEGES tkp;
    tkp.Privileges[0].Luid       = privilege.Data();
    tkp.PrivilegeCount           = 1;
    tkp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;
    if (AdjustTokenPrivileges(_impl->handle, false, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinToken::EnablePrivileges(WinPrivilege::Privilege privilegeName, bool enable)
{
    auto privilege = WinPrivilege::GetPrivilege(privilegeName);
    if (privilege.has_value())
    {
        return EnablePrivileges(*privilege, enable);
    }
    else
    {
        return zeus::unexpected(privilege.error());
    }
}

zeus::expected<bool, std::error_code> WinToken::HasPrivilege(const WinPrivilege::Privilege &privilegeName) const
{
    auto privilege = WinPrivilege::GetPrivilege(privilegeName);
    if (privilege.has_value())
    {
        return HasPrivilege(*privilege);
    }
    else
    {
        return zeus::unexpected(privilege.error());
    }
}

zeus::expected<bool, std::error_code> WinToken::HasPrivilege(const WinPrivilege &privilege) const
{
    auto buffer = GetTokenInfo(_impl->handle, TokenPrivileges);
    if (buffer.has_value())
    {
        auto *privileges = reinterpret_cast<TOKEN_PRIVILEGES *>(buffer->data());
        for (size_t index = 0; index < privileges->PrivilegeCount; ++index)
        {
            if (privileges->Privileges[index].Luid.HighPart == privilege.Data().HighPart &&
                privileges->Privileges[index].Luid.LowPart == privilege.Data().LowPart)

            {
                return (privileges->Privileges[index].Attributes & SE_PRIVILEGE_ENABLED) != 0;
            }
        }
        return false;
    }
    else
    {
        return zeus::unexpected(buffer.error());
    }
}

zeus::expected<WinToken, std::error_code> WinToken::Duplicate(bool primary, SecurityImpersonationLevel level) const
{
    WinToken token;
    if (DuplicateTokenEx(
            _impl->handle, MAXIMUM_ALLOWED, NULL, static_cast<SECURITY_IMPERSONATION_LEVEL>(level), primary ? TokenPrimary : TokenImpersonation,
            &token
        ))
    {
        return token;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<bool, std::error_code> WinToken::HasAdmin() const
{
    auto kBuiltinAdminSID = WinSID::GetWellknowSID(WinBuiltinAdministratorsSid);
    if (!kBuiltinAdminSID.has_value())
    {
        return zeus::unexpected(kBuiltinAdminSID.error());
    }
    auto level = GetIntegrityLevel();
    if (!level.has_value())
    {
        return zeus::unexpected(level.error());
    }
    if (WinToken::WinTokenIntegrityLevel::kHigh != level && WinToken::WinTokenIntegrityLevel::kSystem != level)
    {
        return false;
    }
    BOOL result  = FALSE;
    //只有Impersonation的Token才能使用CheckTokenMembership
    auto primary = IsPrimary();
    if (!primary.has_value())
    {
        return zeus::unexpected(primary.error());
    }
    if (*primary)
    {
        auto duplicateToken = Duplicate(false, WinToken::SecurityImpersonationLevel::Impersonation);
        if (duplicateToken.has_value())
        {
            CheckTokenMembership(*duplicateToken, *kBuiltinAdminSID, &result);
        }
        else
        {
            return zeus::unexpected(duplicateToken.error());
        }
    }
    else
    {
        CheckTokenMembership(*this, *kBuiltinAdminSID, &result);
    }
    return result;
}

WinToken WinToken::GetCurrentProcessToken()
{
    zeus::WinToken token;
    OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &token);
    assert(!token.Empty());
    return token;
}

WinToken WinToken::GetCurrentThreadToken()
{
    zeus::WinToken token;
    if (!OpenThreadToken(GetCurrentThread(), MAXIMUM_ALLOWED, FALSE, &token))
    {
        if (ERROR_NO_TOKEN == GetLastError())
        {
            return GetCurrentProcessToken();
        }
    }
    assert(!token.Empty());
    return token;
}

zeus::expected<bool, std::error_code> WinToken::CurrentProcessHasAdmin()
{
    return GetCurrentProcessToken().HasAdmin();
}
zeus::expected<bool, std::error_code> WinToken::CurrentThreadHasAdmin()
{
    return GetCurrentThreadToken().HasAdmin();
}
zeus::expected<WinToken, std::error_code> WinToken::GetSessionToken(WinSession::SessionId session)
{
    WinToken token;
    if (WTSQueryUserToken(session, &token))
    {
        return token;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<WinToken, std::error_code> WinToken::GetProcessToken(Process::PID pid)
{
    WinHandle handle;
    if (handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid), !handle.Empty())
    {
        return GetProcessToken(handle);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<WinToken, std::error_code> WinToken::GetProcessToken(const Process &process)
{
    return GetProcessToken(process.Id());
}

zeus::expected<WinToken, std::error_code> WinToken::GetProcessToken(HANDLE processHandle)
{
    WinToken token;
    if (OpenProcessToken(processHandle, MAXIMUM_ALLOWED, &token))
    {
        return token;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

} // namespace zeus
#endif
