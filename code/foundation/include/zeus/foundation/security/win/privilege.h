#pragma once
#ifdef _WIN32
#include <memory>
#include <system_error>
#include <string>
#include "zeus/expected.hpp"
#include "zeus/foundation/core/win/win_windef.h"
namespace zeus
{
struct WinPrivilegeImpl;
class WinPrivilege
{
public:
    enum class Privilege
    {
        kSE_CREATE_TOKEN_NAME = 0,
        kSE_ASSIGNPRIMARYTOKEN_NAME,
        kSE_LOCK_MEMORY_NAME,
        kSE_INCREASE_QUOTA_NAME,
        kSE_UNSOLICITED_INPUT_NAME,
        kSE_MACHINE_ACCOUNT_NAME,
        kSE_TCB_NAME,
        kSE_SECURITY_NAME,
        kSE_TAKE_OWNERSHIP_NAME,
        kSE_LOAD_DRIVER_NAME,
        kSE_SYSTEM_PROFILE_NAME,
        kSE_SYSTEMTIME_NAME,
        kSE_PROF_SINGLE_PROCESS_NAME,
        kSE_INC_BASE_PRIORITY_NAME,
        kSE_CREATE_PAGEFILE_NAME,
        kSE_CREATE_PERMANENT_NAME,
        kSE_BACKUP_NAME,
        kSE_RESTORE_NAME,
        kSE_SHUTDOWN_NAME,
        kSE_DEBUG_NAME,
        kSE_AUDIT_NAME,
        kSE_SYSTEM_ENVIRONMENT_NAME,
        kSE_CHANGE_NOTIFY_NAME,
        kSE_REMOTE_SHUTDOWN_NAME,
        kSE_UNDOCK_NAME,
        kSE_SYNC_AGENT_NAME,
        kSE_ENABLE_DELEGATION_NAME,
        kSE_MANAGE_VOLUME_NAME,
        kSE_IMPERSONATE_NAME,
        kSE_CREATE_GLOBAL_NAME,
        kSE_TRUSTED_CREDMAN_ACCESS_NAME,
        kSE_RELABEL_NAME,
        kSE_INC_WORKING_SET_NAME,
        kSE_TIME_ZONE_NAME,
        kSE_CREATE_SYMBOLIC_LINK_NAME,
        kSE_DELEGATE_SESSION_USER_IMPERSONATE_NAME
    };
    WinPrivilege();
    WinPrivilege(const LUID &luid) noexcept;
    ~WinPrivilege();
    WinPrivilege(const WinPrivilege &other) noexcept;
    WinPrivilege(WinPrivilege &&other) noexcept;
    WinPrivilege &operator=(const WinPrivilege &other) noexcept;
    WinPrivilege &operator=(WinPrivilege &&other) noexcept;
    LUID         *operator&() noexcept;
    WinPrivilege &operator=(const LUID &luid) noexcept;
    operator LUID() const noexcept;
    operator LUID *() const noexcept;
    bool operator==(const LUID &luid) const noexcept;
    bool operator==(const WinPrivilege &other) const noexcept;
    LUID Data() const noexcept;

    zeus::expected<std::string, std::error_code>  GetName();
    zeus::expected<std::wstring, std::error_code> GetWName();
public:
    static zeus::expected<WinPrivilege, std::error_code> GetPrivilege(Privilege name);
    static zeus::expected<WinPrivilege, std::error_code> GetPrivilege(const std::string &name);
    static zeus::expected<WinPrivilege, std::error_code> GetPrivilege(const std::wstring &name);
    static std::string                                   CastPrivilegeName(Privilege name);
    static std::wstring                                  CastPrivilegeWName(Privilege name);
private:
    std::unique_ptr<WinPrivilegeImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
