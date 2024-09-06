#ifdef _WIN32
#include "zeus/foundation/security/win/impersonate.h"
#include <Windows.h>
#include <Wtsapi32.h>
#include "zeus/foundation/system/win/session.h"
#include "zeus/foundation/resource/win/handle.h"

#pragma comment(lib, "WtsApi32.lib")
namespace zeus
{
struct WinImpersonateActiveUserImpl
{
    bool impersonate = false;
};

WinImpersonateActiveUser::WinImpersonateActiveUser() : _impl(std::make_unique<WinImpersonateActiveUserImpl>())
{
    auto sessions = WinSession::ListId(WinSession::SessionState::Active);
    if (!sessions.empty())
    {
        const DWORD sessionId = sessions.front();
        WinHandle   userToken;
        WinHandle   fakeToken;

        if (WTSQueryUserToken(sessionId, &userToken))
        {
            if (DuplicateTokenEx(userToken, TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS, 0, SecurityImpersonation, TokenPrimary, &fakeToken) == TRUE)
            {
                if (ImpersonateLoggedOnUser(fakeToken))
                {
                    _impl->impersonate = true;
                }
            }
        }
    }
}
WinImpersonateActiveUser::~WinImpersonateActiveUser()
{
    if (_impl->impersonate)
    {
        RevertToSelf();
    }
}
WinImpersonateActiveUser::operator bool() const noexcept
{
    return _impl->impersonate;
}

struct WinImpersonateTokenImpl
{
    bool impersonate = false;
};

WinImpersonateToken::WinImpersonateToken(const WinToken& token) : _impl(std::make_unique<WinImpersonateTokenImpl>())
{
    if (ImpersonateLoggedOnUser(token))
    {
        _impl->impersonate = true;
    }
}
WinImpersonateToken::~WinImpersonateToken()
{
    if (_impl->impersonate)
    {
        RevertToSelf();
    }
}
WinImpersonateToken::operator bool() const noexcept
{
    return _impl->impersonate;
}
} // namespace zeus
#endif
