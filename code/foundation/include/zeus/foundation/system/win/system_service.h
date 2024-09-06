#pragma once
#ifdef _WIN32
#include <memory>
#include <string>
#include <chrono>
#include <optional>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{
struct WinSysterServiceFailureActionImpl;
class WinSysterServiceFailureAction
{
public:
    enum class FailureActionType
    {
        kNone,
        kRestart,
        kReboot,
        kRunCommand,
    };
    WinSysterServiceFailureAction();
    WinSysterServiceFailureAction(const WinSysterServiceFailureAction&) = delete;
    WinSysterServiceFailureAction(WinSysterServiceFailureAction&& other) noexcept;
    WinSysterServiceFailureAction& operator=(const WinSysterServiceFailureAction&) = delete;
    WinSysterServiceFailureAction& operator=(WinSysterServiceFailureAction&& other) noexcept;
    ~WinSysterServiceFailureAction();
    std::optional<std::chrono::seconds>                                  GetResetPeriod() const;
    std::string                                                          GetRebootMessage() const;
    std::string                                                          GetCommand() const;
    std::vector<std::pair<FailureActionType, std::chrono::milliseconds>> GetActions() const;
    void                                                                 SetResetPeriod(const std::optional<std::chrono::seconds>& period);
    void                                                                 SetRebootMessage(const std::string& message);
    void                                                                 SetCommand(const std::string& command);
    void                                                                 AddAction(FailureActionType action, const std::chrono::milliseconds& delay);
    void                                                                 ClearActions();
    friend class WinSystemService;
private:
    std::unique_ptr<WinSysterServiceFailureActionImpl> _impl;
};

struct WinSystemServiceImpl;
class WinSystemService
{
public:
    enum class StartupType
    {
        kAutoStart,
        kBootStart,
        kSystemStart,
        kManualStart,
        kDisable,
    };

    enum class ServiceType
    {
        kKernelDriver,
        kFileSystemDriver,
        kAdapter,
        kRecognizerDriver,
        kWin32OwnProcess,
        kWin32ShareProcess,
        kUserService,
        kUserServiceInstance,
        kInteractiveProcess,
        kPkgService,
    };
    enum class ServiceState
    {
        kStoped,
        kRunning,
        kPaused,
        kStartPending,
        kStopPending,
        kContinuePending,
        kPausePending,
    };
    WinSystemService(const WinSystemService&) = delete;
    WinSystemService(WinSystemService&& other) noexcept;
    WinSystemService& operator=(const WinSystemService&) = delete;
    WinSystemService& operator=(WinSystemService&& other) noexcept;
    ~WinSystemService();
    std::string                                                    Name() const;
    zeus::expected<std::string, std::error_code>                   GetDisplayName() const;
    zeus::expected<std::string, std::error_code>                   GetPath() const;
    zeus::expected<ServiceType, std::error_code>                   GetServiceType() const;
    zeus::expected<StartupType, std::error_code>                   GetStartupType() const;
    zeus::expected<ServiceState, std::error_code>                  GetServiceState() const;
    zeus::expected<std::string, std::error_code>                   GetDescription() const;
    zeus::expected<bool, std::error_code>                          IsDelayAutoStart() const;
    zeus::expected<std::vector<std::string>, std::error_code>      GetDependencies() const;
    zeus::expected<std::string, std::error_code>                   GetGroup() const;
    zeus::expected<uint32_t, std::error_code>                      GetGroupTag() const;
    zeus::expected<WinSysterServiceFailureAction, std::error_code> GetFailureAction() const;

    zeus::expected<void, std::error_code> SetDisplayName(const std::string& displayName);
    zeus::expected<void, std::error_code> SetPath(const std::string& path);
    zeus::expected<void, std::error_code> SetServiceType(ServiceType type);
    zeus::expected<void, std::error_code> SetStartupType(StartupType type);
    zeus::expected<void, std::error_code> SetDescription(const std::string& description);
    zeus::expected<void, std::error_code> SetDelayAutoStart(bool delay);
    zeus::expected<void, std::error_code> SetDependencies(const std::vector<std::string>& dependencies);
    zeus::expected<void, std::error_code> SetGroup(const std::string& group);
    zeus::expected<void, std::error_code> SetGroupTag(uint32_t tag);
    zeus::expected<void, std::error_code> SetFailureAction(const WinSysterServiceFailureAction& action);

    zeus::expected<void, std::error_code> SetBaseConfig(
        ServiceType serviceType, StartupType startType, const std::string& path, const std::string& displayName
    );

    zeus::expected<void, std::error_code> Start(const std::vector<std::string>& params);
    zeus::expected<void, std::error_code> Stop();
    zeus::expected<void, std::error_code> Remove();
public:
    static zeus::expected<WinSystemService, std::error_code> Open(const std::string& name, bool control = true, bool change = true);
    static zeus::expected<WinSystemService, std::error_code> Create(
        const std::string& name, ServiceType serviceType, StartupType startType, const std::string& path, const std::string& displayName
    );

protected:
    WinSystemService();
private:
    std::unique_ptr<WinSystemServiceImpl> _impl;
};
} // namespace zeus

#endif
#include "zeus/foundation/core/zeus_compatible.h"
