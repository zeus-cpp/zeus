#include "zeus/foundation/system/win/session.h"
#ifdef _WIN32
#include <functional>
#include <cassert>
#include <Windows.h>
#include <Wtsapi32.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/system/shared_library.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/time/time_utils.h"
#include "winsta.h"
#pragma comment(lib, "WtsApi32.lib")

namespace zeus
{

namespace
{

zeus::expected<WINSTATIONINFORMATION, std::error_code> GetStationInfo(ULONG logonId)
{
    typedef BOOLEAN(WINAPI * WinStationQueryInformation)(
        _In_opt_ HANDLE ServerHandle, _In_ ULONG SessionId, _In_ WINSTATIONINFOCLASS WinStationInformationClass,
        _Out_writes_bytes_(WinStationInformationLength) PVOID pWinStationInformation, _In_ ULONG WinStationInformationLength,
        _Out_ PULONG pReturnLength
    );

    static auto winStationQueryInformation =
        zeus::SharedLibrary::Load(std::string("Winsta.dll"))
            .transform([](const zeus::SharedLibrary &library)
                       { return library.GetTypeSymbol<WinStationQueryInformation>("WinStationQueryInformationW"); })
            .value_or(nullptr);
    if (!winStationQueryInformation)
    {
        return zeus::unexpected(make_error_code(SystemError {ERROR_PROC_NOT_FOUND}));
    }
    DWORD                 length = 0;
    WINSTATIONINFORMATION info {};
    if (winStationQueryInformation(nullptr, logonId, WinStationInformation, &info, sizeof(WINSTATIONINFORMATION), &length))
    {
        return info;
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

void EnumerateSessions(std::function<bool(PWTS_SESSION_INFOW info)> &&handler)
{
    PWTS_SESSION_INFOW pwsi  = nullptr;
    DWORD              count = 0;
    zeus::AutoRelease  info(
        [&pwsi]()
        {
            if (pwsi)
            {
                WTSFreeMemory(pwsi);
            }
        }
    );
    if (WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pwsi, &count))
    {
        for (DWORD index = 0; index < count; ++index)
        {
            if (!handler(pwsi + index))
            {
                break;
            }
        }
    }
}

WinSession::SessionState CastState(WTS_CONNECTSTATE_CLASS state)
{
    switch (state)
    {
    case WTS_CONNECTSTATE_CLASS::WTSActive:
        return WinSession::SessionState::Active;
    case WTS_CONNECTSTATE_CLASS::WTSConnected:
        return WinSession::SessionState::Connected;
    case WTS_CONNECTSTATE_CLASS::WTSConnectQuery:
        return WinSession::SessionState::ConnectQuery;
    case WTS_CONNECTSTATE_CLASS::WTSShadow:
        return WinSession::SessionState::Shadow;
    case WTS_CONNECTSTATE_CLASS::WTSDisconnected:
        return WinSession::SessionState::Disconnected;
    case WTS_CONNECTSTATE_CLASS::WTSIdle:
        return WinSession::SessionState::Idle;
    case WTS_CONNECTSTATE_CLASS::WTSListen:
        return WinSession::SessionState::Listen;
    case WTS_CONNECTSTATE_CLASS::WTSReset:
        return WinSession::SessionState::Reset;
    case WTS_CONNECTSTATE_CLASS::WTSDown:
        return WinSession::SessionState::Down;
    case WTS_CONNECTSTATE_CLASS::WTSInit:
        return WinSession::SessionState::Init;
    default:
        assert(false);
        return WinSession::SessionState::Init;
    }
}

WTS_CONNECTSTATE_CLASS CastState(WinSession::SessionState state)
{
    switch (state)
    {
    case WinSession::SessionState::Active:
        return WTS_CONNECTSTATE_CLASS::WTSActive;
    case WinSession::SessionState::Connected:
        return WTS_CONNECTSTATE_CLASS::WTSConnected;
    case WinSession::SessionState::ConnectQuery:
        return WTS_CONNECTSTATE_CLASS::WTSConnectQuery;
    case WinSession::SessionState::Shadow:
        return WTS_CONNECTSTATE_CLASS::WTSShadow;
    case WinSession::SessionState::Disconnected:
        return WTS_CONNECTSTATE_CLASS::WTSDisconnected;
    case WinSession::SessionState::Idle:
        return WTS_CONNECTSTATE_CLASS::WTSIdle;
    case WinSession::SessionState::Listen:
        return WTS_CONNECTSTATE_CLASS::WTSListen;
    case WinSession::SessionState::Reset:
        return WTS_CONNECTSTATE_CLASS::WTSReset;
    case WinSession::SessionState::Down:
        return WTS_CONNECTSTATE_CLASS::WTSDown;
    case WinSession::SessionState::Init:
        return WTS_CONNECTSTATE_CLASS::WTSInit;
    default:
        assert(false);
        return WTS_CONNECTSTATE_CLASS::WTSInit;
    }
}

} // namespace

struct SessionInfoImpl
{
    std::string                                          domain;
    std::string                                          username;
    std::optional<std::chrono::system_clock::time_point> logonTime;
    std::optional<std::chrono::system_clock::time_point> connectTime;
    std::optional<std::chrono::system_clock::time_point> disconnectTime;
    std::optional<std::chrono::system_clock::time_point> lastInputTime;
};

SessionInfo::SessionInfo() : _impl(std::make_unique<SessionInfoImpl>())
{
}

SessionInfo::~SessionInfo()
{
}

SessionInfo::SessionInfo(SessionInfo &&other) noexcept : _impl(std::move(other._impl))
{
}

SessionInfo &SessionInfo::operator=(SessionInfo &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

std::string SessionInfo::Username() const
{
    return _impl->username;
}

std::string SessionInfo::DomainName() const
{
    return _impl->domain;
}

std::optional<std::chrono::system_clock::time_point> SessionInfo::LogonTime() const noexcept
{
    return _impl->logonTime;
}

std::optional<std::chrono::system_clock::time_point> SessionInfo::ConnectTime() const noexcept
{
    return _impl->connectTime;
}

std::optional<std::chrono::system_clock::time_point> SessionInfo::DisconnectTime() const noexcept
{
    return _impl->disconnectTime;
}

std::optional<std::chrono::system_clock::time_point> SessionInfo::LastInputTime() const noexcept
{
    return _impl->lastInputTime;
}

struct WinSessionImpl
{
    WinSession::SessionId    id;
    WinSession::SessionState state;
    std::string              name;
};

WinSession::WinSession() : _impl(std::make_unique<WinSessionImpl>())
{
}

WinSession::~WinSession()
{
}

WinSession::WinSession(WinSession &&other) noexcept : _impl(std::move(other._impl))
{
}

WinSession &WinSession::operator=(WinSession &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

WinSession::SessionId WinSession::Id() const
{
    return _impl->id;
}

WinSession::SessionState WinSession::State() const
{
    return _impl->state;
}
std::string WinSession::WinStationName() const
{
    return _impl->name;
}

std::vector<WinSession::SessionId> WinSession::ListAllId()
{
    std::vector<SessionId> result;
    EnumerateSessions(
        [&result](PWTS_SESSION_INFOW info)
        {
            result.emplace_back(info->SessionId);
            return true;
        }
    );
    return result;
}

std::vector<WinSession::SessionId> WinSession::ListId(SessionState state)
{
    std::vector<SessionId> result;
    EnumerateSessions(
        [&result, state](PWTS_SESSION_INFOW info)
        {
            if (state == CastState(info->State))
            {
                result.emplace_back(info->SessionId);
            }
            return true;
        }
    );
    return result;
}

std::optional<WinSession::SessionId> WinSession::GetConsoleId()
{
    if (const auto id = WTSGetActiveConsoleSessionId(); 0xFFFFFFFF == id)
    {
        return id;
    }
    else
    {
        return std::nullopt;
    }
}
std::vector<WinSession> WinSession::ListAll()
{
    std::vector<WinSession> sessions;
    EnumerateSessions(
        [&sessions](PWTS_SESSION_INFOW info)
        {
            WinSession session;
            session._impl->id    = info->SessionId;
            session._impl->state = CastState(info->State);
            session._impl->name  = zeus::CharsetUtils::UnicodeToUTF8(info->pWinStationName);
            sessions.emplace_back(std::move(session));
            return true;
        }
    );
    return sessions;
}
std::vector<WinSession> WinSession::List(SessionState state)
{
    std::vector<WinSession> sessions;
    EnumerateSessions(
        [&sessions, &state](PWTS_SESSION_INFOW info)
        {
            if (state != CastState(info->State))
            {
                return true;
            }
            WinSession session;
            session._impl->id    = info->SessionId;
            session._impl->state = CastState(info->State);
            session._impl->name  = zeus::CharsetUtils::UnicodeToUTF8(info->pWinStationName);
            sessions.emplace_back(std::move(session));
            return true;
        }
    );
    return sessions;
}
std::optional<WinSession> WinSession::GetConsole()
{
    std::optional<WinSession> session;
    if (const auto id = WTSGetActiveConsoleSessionId(); 0xFFFFFFFF != id)
    {
        EnumerateSessions(
            [&id, &session](PWTS_SESSION_INFOW info)
            {
                if (id == info->SessionId)
                {
                    session               = WinSession();
                    session->_impl->id    = info->SessionId;
                    session->_impl->state = CastState(info->State);
                    session->_impl->name  = zeus::CharsetUtils::UnicodeToUTF8(info->pWinStationName);
                    return false;
                }
                return true;
            }
        );
    }
    return session;
}

zeus::expected<SessionInfo, std::error_code> WinSession::GetSessionInfo(SessionId id)
{
    auto info = GetStationInfo(id);
    if (info.has_value())
    {
        SessionInfo sessionInfo;
        sessionInfo._impl->domain   = zeus::CharsetUtils::UnicodeToUTF8(info->Domain);
        sessionInfo._impl->username = zeus::CharsetUtils::UnicodeToUTF8(info->UserName);
        if (info->LogonTime.QuadPart)
        {
            sessionInfo._impl->logonTime = zeus::FiletimeToSystemTime(info->LogonTime);
        }
        if (info->ConnectTime.QuadPart)
        {
            sessionInfo._impl->connectTime = zeus::FiletimeToSystemTime(info->ConnectTime);
        }
        if (info->DisconnectTime.QuadPart)
        {
            sessionInfo._impl->disconnectTime = zeus::FiletimeToSystemTime(info->DisconnectTime);
        }
        if (info->LastInputTime.QuadPart)
        {
            sessionInfo._impl->lastInputTime = zeus::FiletimeToSystemTime(info->LastInputTime);
        }
        return sessionInfo;
    }
    else
    {
        return zeus::unexpected(info.error());
    }
}

} // namespace zeus

#endif
