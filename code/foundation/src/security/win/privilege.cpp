#include "zeus/foundation/security/win/privilege.h"
#ifdef _WIN32
#include <string>
#include <cassert>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/string/charset_utils.h"
namespace zeus
{
namespace
{
std::wstring PrivilegeName(WinPrivilege::Privilege name)
{
    switch (name)
    {
    case WinPrivilege::Privilege::kSE_CREATE_TOKEN_NAME:
        return SE_CREATE_TOKEN_NAME;
    case WinPrivilege::Privilege::kSE_ASSIGNPRIMARYTOKEN_NAME:
        return SE_ASSIGNPRIMARYTOKEN_NAME;
    case WinPrivilege::Privilege::kSE_LOCK_MEMORY_NAME:
        return SE_LOCK_MEMORY_NAME;
    case WinPrivilege::Privilege::kSE_INCREASE_QUOTA_NAME:
        return SE_INCREASE_QUOTA_NAME;
    case WinPrivilege::Privilege::kSE_UNSOLICITED_INPUT_NAME:
        return SE_UNSOLICITED_INPUT_NAME;
    case WinPrivilege::Privilege::kSE_MACHINE_ACCOUNT_NAME:
        return SE_MACHINE_ACCOUNT_NAME;
    case WinPrivilege::Privilege::kSE_TCB_NAME:
        return SE_TCB_NAME;
    case WinPrivilege::Privilege::kSE_SECURITY_NAME:
        return SE_SECURITY_NAME;
    case WinPrivilege::Privilege::kSE_TAKE_OWNERSHIP_NAME:
        return SE_TAKE_OWNERSHIP_NAME;
    case WinPrivilege::Privilege::kSE_LOAD_DRIVER_NAME:
        return SE_LOAD_DRIVER_NAME;
    case WinPrivilege::Privilege::kSE_SYSTEM_PROFILE_NAME:
        return SE_SYSTEM_PROFILE_NAME;
    case WinPrivilege::Privilege::kSE_SYSTEMTIME_NAME:
        return SE_SYSTEMTIME_NAME;
    case WinPrivilege::Privilege::kSE_PROF_SINGLE_PROCESS_NAME:
        return SE_PROF_SINGLE_PROCESS_NAME;
    case WinPrivilege::Privilege::kSE_INC_BASE_PRIORITY_NAME:
        return SE_INC_BASE_PRIORITY_NAME;
    case WinPrivilege::Privilege::kSE_CREATE_PAGEFILE_NAME:
        return SE_CREATE_PAGEFILE_NAME;
    case WinPrivilege::Privilege::kSE_CREATE_PERMANENT_NAME:
        return SE_CREATE_PERMANENT_NAME;
    case WinPrivilege::Privilege::kSE_BACKUP_NAME:
        return SE_BACKUP_NAME;
    case WinPrivilege::Privilege::kSE_RESTORE_NAME:
        return SE_RESTORE_NAME;
    case WinPrivilege::Privilege::kSE_SHUTDOWN_NAME:
        return SE_SHUTDOWN_NAME;
    case WinPrivilege::Privilege::kSE_DEBUG_NAME:
        return SE_DEBUG_NAME;
    case WinPrivilege::Privilege::kSE_AUDIT_NAME:
        return SE_AUDIT_NAME;
    case WinPrivilege::Privilege::kSE_SYSTEM_ENVIRONMENT_NAME:
        return SE_SYSTEM_ENVIRONMENT_NAME;
    case WinPrivilege::Privilege::kSE_CHANGE_NOTIFY_NAME:
        return SE_CHANGE_NOTIFY_NAME;
    case WinPrivilege::Privilege::kSE_REMOTE_SHUTDOWN_NAME:
        return SE_REMOTE_SHUTDOWN_NAME;
    case WinPrivilege::Privilege::kSE_UNDOCK_NAME:
        return SE_UNDOCK_NAME;
    case WinPrivilege::Privilege::kSE_SYNC_AGENT_NAME:
        return SE_SYNC_AGENT_NAME;
    case WinPrivilege::Privilege::kSE_ENABLE_DELEGATION_NAME:
        return SE_ENABLE_DELEGATION_NAME;
    case WinPrivilege::Privilege::kSE_MANAGE_VOLUME_NAME:
        return SE_MANAGE_VOLUME_NAME;
    case WinPrivilege::Privilege::kSE_IMPERSONATE_NAME:
        return SE_IMPERSONATE_NAME;
    case WinPrivilege::Privilege::kSE_CREATE_GLOBAL_NAME:
        return SE_CREATE_GLOBAL_NAME;
    case WinPrivilege::Privilege::kSE_TRUSTED_CREDMAN_ACCESS_NAME:
        return SE_TRUSTED_CREDMAN_ACCESS_NAME;
    case WinPrivilege::Privilege::kSE_RELABEL_NAME:
        return SE_RELABEL_NAME;
    case WinPrivilege::Privilege::kSE_INC_WORKING_SET_NAME:
        return SE_INC_WORKING_SET_NAME;
    case WinPrivilege::Privilege::kSE_TIME_ZONE_NAME:
        return SE_TIME_ZONE_NAME;
    case WinPrivilege::Privilege::kSE_CREATE_SYMBOLIC_LINK_NAME:
        return SE_CREATE_SYMBOLIC_LINK_NAME;
    case WinPrivilege::Privilege::kSE_DELEGATE_SESSION_USER_IMPERSONATE_NAME:
        return TEXT("SeDelegateSessionUserImpersonatePrivilege");
    default:
        assert(false);
    }
    return L"";
}
} // namespace

struct WinPrivilegeImpl
{
    LUID luid = {};
};

WinPrivilege::WinPrivilege() : _impl(std::make_unique<WinPrivilegeImpl>())
{
}

WinPrivilege::WinPrivilege(const LUID& luid) noexcept : _impl(std::make_unique<WinPrivilegeImpl>())
{
    _impl->luid = luid;
}

WinPrivilege::~WinPrivilege()
{
}

WinPrivilege::WinPrivilege(const WinPrivilege& other) noexcept : _impl(std::make_unique<WinPrivilegeImpl>(*other._impl))
{
}

WinPrivilege::WinPrivilege(WinPrivilege&& other) noexcept : _impl(std::move(other._impl))
{
}

WinPrivilege& WinPrivilege::operator=(const WinPrivilege& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

WinPrivilege& WinPrivilege::operator=(WinPrivilege&& other) noexcept
{
    if (this != std::addressof(other))
    {
        _impl.swap(other._impl);
    }
    return *this;
}

LUID* WinPrivilege::operator&() noexcept
{
    return &_impl->luid;
}

WinPrivilege::operator LUID() const noexcept
{
    return _impl->luid;
}

WinPrivilege::operator LUID*() const noexcept
{
    return &_impl->luid;
}

bool WinPrivilege::operator==(const LUID& luid) const noexcept
{
    return _impl->luid.HighPart == luid.HighPart && _impl->luid.LowPart == luid.LowPart;
}

bool WinPrivilege::operator==(const WinPrivilege& other) const noexcept
{
    return _impl->luid.HighPart == other._impl->luid.HighPart && _impl->luid.LowPart == other._impl->luid.LowPart;
}

WinPrivilege& WinPrivilege::operator=(const LUID& luid) noexcept
{
    _impl->luid = luid;
    return *this;
}

LUID WinPrivilege::Data() const noexcept
{
    return _impl->luid;
}

zeus::expected<std::string, std::error_code> WinPrivilege::GetName()
{
    DWORD size = 0;
    LookupPrivilegeNameA(nullptr, &_impl->luid, nullptr, &size);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        auto data = std::make_unique<char[]>(size);
        if (LookupPrivilegeNameA(nullptr, &_impl->luid, data.get(), &size))
        {
            return std::string(data.get(), data.get() + size);
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

zeus::expected<std::wstring, std::error_code> WinPrivilege::GetWName()
{
    DWORD size = 0;
    LookupPrivilegeNameW(nullptr, &_impl->luid, nullptr, &size);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        auto data = std::make_unique<wchar_t[]>(size);
        if (LookupPrivilegeNameW(nullptr, &_impl->luid, data.get(), &size))
        {
            return std::wstring(data.get(), data.get() + size);
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

zeus::expected<WinPrivilege, std::error_code> WinPrivilege::GetPrivilege(Privilege name)
{
    return GetPrivilege(PrivilegeName(name));
}

zeus::expected<WinPrivilege, std::error_code> WinPrivilege::GetPrivilege(const std::string& name)
{
    return GetPrivilege(CharsetUtils::UTF8ToUnicode(name));
}

zeus::expected<WinPrivilege, std::error_code> WinPrivilege::GetPrivilege(const std::wstring& name)
{
    LUID luid;
    if (LookupPrivilegeValueW(nullptr, name.c_str(), &luid))
    {
        return WinPrivilege(luid);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

std::string WinPrivilege::CastPrivilegeName(Privilege name)
{
    return CharsetUtils::UnicodeToUTF8(PrivilegeName(name));
}

std::wstring WinPrivilege::CastPrivilegeWName(Privilege name)
{
    return PrivilegeName(name);
}
} // namespace zeus
#endif
