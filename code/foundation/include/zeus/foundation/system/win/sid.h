#pragma once

#ifdef _WIN32
#include <string>
#include <system_error>
#include <utility>
#include <memory>
#include "zeus/expected.hpp"
#include "zeus/foundation/core/win/win_windef.h"

namespace zeus
{
class WinSID;
struct WinAccountImpl;
class WinAccount
{
public:
    WinAccount(const std::string& domain, const std::string& username);
    ~WinAccount();
    WinAccount(const WinAccount& other) noexcept;
    WinAccount(WinAccount&& other) noexcept;
    WinAccount& operator=(const WinAccount& other) noexcept;
    WinAccount& operator=(WinAccount&& other) noexcept;
    bool        operator==(const WinAccount& other) const noexcept;
    bool        operator!=(const WinAccount& other) const noexcept;
    std::string Username() const;
    std::string Domain() const;
    std::string ToString() const;
public:
    friend class WinSID;
private:
    std::unique_ptr<WinAccountImpl> _impl;
};

struct WinSIDImpl;
class WinSID
{
    /*
    构造函数隐含着所有权的转移，PSID是一种句柄资源，但是其释放方式是不固定的，转移所有权后无法释放，所以不提供PSID的构造函数，只提供从PISD拷贝处WinSID的静态函数
    */
public:
    WinSID(size_t sidLength);
    ~WinSID();
    WinSID(const WinSID&) = delete;
    WinSID(WinSID&& other) noexcept;
    WinSID& operator=(const WinSID&) = delete;
    WinSID& operator=(WinSID&& other) noexcept;
    operator PSID() const noexcept;
    bool                                         operator==(const WinSID& other) const noexcept;
    bool                                         operator!=(const WinSID& other) const noexcept;
    bool                                         operator==(PSID other) const noexcept;
    bool                                         operator!=(PSID other) const noexcept;
    size_t                                       Length() const noexcept;
    PSID                                         Psid() const noexcept;
    zeus::expected<WinAccount, std::error_code>  GetAccount() const;
    zeus::expected<std::string, std::error_code> GetString() const;

    static zeus::expected<WinAccount, std::error_code>  GetSIDAccount(PSID sid);
    static zeus::expected<std::string, std::error_code> GetSIDString(PSID sid);
    static zeus::expected<WinSID, std::error_code>      CopySID(PSID sid);
    static zeus::expected<WinSID, std::error_code>      GetAccountSID(const std::string& username);
    static zeus::expected<WinSID, std::error_code>      GetWellknowSID(WELL_KNOWN_SID_TYPE type);
    static bool                                         IsSIDEqual(PSID sid1, PSID sid2) noexcept;
private:
    std::unique_ptr<WinSIDImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
