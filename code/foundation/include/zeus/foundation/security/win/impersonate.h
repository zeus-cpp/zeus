#pragma once
#ifdef _WIN32
#include <memory>
#include "zeus/foundation/security/win/token.h"
namespace zeus
{
//此类仅在服务下生效
struct WinImpersonateActiveUserImpl;
class WinImpersonateActiveUser
{
public:
    WinImpersonateActiveUser();
    ~WinImpersonateActiveUser();
    WinImpersonateActiveUser(const WinImpersonateActiveUser&)            = delete;
    WinImpersonateActiveUser(WinImpersonateActiveUser&& other)           = delete;
    WinImpersonateActiveUser& operator=(const WinImpersonateActiveUser&) = delete;
    WinImpersonateActiveUser& operator=(WinImpersonateActiveUser&&)      = delete;
    operator bool() const noexcept;
private:
    std::unique_ptr<WinImpersonateActiveUserImpl> _impl;
};

struct WinImpersonateTokenImpl;
class WinImpersonateToken
{
public:
    WinImpersonateToken(const WinToken& token);
    ~WinImpersonateToken();
    WinImpersonateToken(const WinImpersonateToken&)            = delete;
    WinImpersonateToken(WinImpersonateToken&& other)           = delete;
    WinImpersonateToken& operator=(const WinImpersonateToken&) = delete;
    WinImpersonateToken& operator=(WinImpersonateToken&&)      = delete;
    operator bool() const noexcept;
private:
    std::unique_ptr<WinImpersonateTokenImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
