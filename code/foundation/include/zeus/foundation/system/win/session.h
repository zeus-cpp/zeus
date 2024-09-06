#pragma once
#ifdef _WIN32
#include <vector>
#include <memory>
#include <chrono>
#include <optional>
#include <string>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{

struct SessionInfoImpl;
class SessionInfo
{
public:
    ~SessionInfo();
    SessionInfo(const SessionInfo&) = delete;
    SessionInfo(SessionInfo&& other) noexcept;
    SessionInfo&                                         operator=(const SessionInfo&) = delete;
    SessionInfo&                                         operator=(SessionInfo&& other) noexcept;
    std::string                                          Username() const;
    std::string                                          DomainName() const;
    std::optional<std::chrono::system_clock::time_point> LogonTime() const noexcept;
    std::optional<std::chrono::system_clock::time_point> ConnectTime() const noexcept;
    std::optional<std::chrono::system_clock::time_point> DisconnectTime() const noexcept;
    std::optional<std::chrono::system_clock::time_point> LastInputTime() const noexcept;
    friend class WinSession;
protected:
    SessionInfo();
private:
    std::unique_ptr<SessionInfoImpl> _impl;
};

struct WinSessionImpl;
class WinSession
{
public:
    using SessionId = unsigned long;
    enum class SessionState
    {
        Active = 0,   // User logged on to WinStation
        Connected,    // WinStation connected to client
        ConnectQuery, // In the process of connecting to client
        Shadow,       // Shadowing another WinStation
        Disconnected, // WinStation logged on without client
        Idle,         // Waiting for client to connect
        Listen,       // WinStation is listening for connection
        Reset,        // WinStation is being reset
        Down,         // WinStation is down due to error
        Init,
    };
    ~WinSession();
    WinSession(const WinSession&) = delete;
    WinSession(WinSession&& other) noexcept;
    WinSession&  operator=(const WinSession&) = delete;
    WinSession&  operator=(WinSession&& other) noexcept;
    SessionId    Id() const;
    SessionState State() const;
    std::string  WinStationName() const;
public:
    static std::vector<SessionId>   ListAllId();
    static std::vector<SessionId>   ListId(SessionState state);
    static std::optional<SessionId> GetConsoleId();

    static std::vector<WinSession>                      ListAll();
    static std::vector<WinSession>                      List(SessionState state);
    static std::optional<WinSession>                    GetConsole();
    static zeus::expected<SessionInfo, std::error_code> GetSessionInfo(SessionId id);
protected:
    WinSession();
private:
    std::unique_ptr<WinSessionImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
