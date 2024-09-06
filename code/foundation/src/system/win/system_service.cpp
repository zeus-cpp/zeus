#include "zeus/foundation/system/win/system_service.h"
#ifdef _WIN32
#include <cassert>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/win/service_handle.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{
struct WinSystemServiceImpl
{
    WinServiceHandle handle;
    std::wstring     serviceName;
};

struct WinSysterServiceFailureActionImpl
{
    std::wstring             rebootMessage;
    std::wstring             command;
    SERVICE_FAILURE_ACTIONSW failAction = {};
    std::vector<SC_ACTION>   actions;
};

namespace
{

WinSystemService::StartupType CastStartupType(DWORD type)
{
    switch (type)
    {
    case SERVICE_AUTO_START:
        return WinSystemService::StartupType::kAutoStart;
    case SERVICE_BOOT_START:
        return WinSystemService::StartupType::kBootStart;
    case SERVICE_SYSTEM_START:
        return WinSystemService::StartupType::kSystemStart;
    case SERVICE_DEMAND_START:
        return WinSystemService::StartupType::kManualStart;
    case SERVICE_DISABLED:
        return WinSystemService::StartupType::kDisable;
    default:
        assert(false);
        return WinSystemService::StartupType::kDisable;
    }
}

DWORD CastStartupType(WinSystemService::StartupType type)
{
    switch (type)
    {
    case WinSystemService::StartupType::kAutoStart:
        return SERVICE_AUTO_START;
    case WinSystemService::StartupType::kBootStart:
        return SERVICE_BOOT_START;
    case WinSystemService::StartupType::kSystemStart:
        return SERVICE_SYSTEM_START;
    case WinSystemService::StartupType::kManualStart:
        return SERVICE_DEMAND_START;
    case WinSystemService::StartupType::kDisable:
        return SERVICE_DISABLED;
    default:
        assert(false);
        return SERVICE_DISABLED;
    }
}

WinSystemService::ServiceType CastServiceType(DWORD type)
{
    switch (type)
    {
    case SERVICE_FILE_SYSTEM_DRIVER:
        return WinSystemService::ServiceType::kFileSystemDriver;
    case SERVICE_KERNEL_DRIVER:
        return WinSystemService::ServiceType::kKernelDriver;
    case SERVICE_ADAPTER:
        return WinSystemService::ServiceType::kAdapter;
    case SERVICE_RECOGNIZER_DRIVER:
        return WinSystemService::ServiceType::kRecognizerDriver;
    case SERVICE_WIN32_OWN_PROCESS:
        return WinSystemService::ServiceType::kWin32OwnProcess;
    case SERVICE_WIN32_SHARE_PROCESS:
        return WinSystemService::ServiceType::kWin32ShareProcess;
    case SERVICE_USER_SERVICE:
        return WinSystemService::ServiceType::kUserService;
    case SERVICE_USERSERVICE_INSTANCE:
        return WinSystemService::ServiceType::kUserServiceInstance;
    case SERVICE_INTERACTIVE_PROCESS:
        return WinSystemService::ServiceType::kInteractiveProcess;
    case SERVICE_PKG_SERVICE:
        return WinSystemService::ServiceType::kPkgService;
    default:
        assert(false);                                          // 未知的服务类型
        return WinSystemService::ServiceType::kWin32OwnProcess; // 默认返回值
    }
}

DWORD CastServiceType(WinSystemService::ServiceType type)
{
    switch (type)
    {
    case WinSystemService::ServiceType::kFileSystemDriver:
        return SERVICE_FILE_SYSTEM_DRIVER;
    case WinSystemService::ServiceType::kKernelDriver:
        return SERVICE_KERNEL_DRIVER;
    case WinSystemService::ServiceType::kAdapter:
        return SERVICE_ADAPTER;
    case WinSystemService::ServiceType::kRecognizerDriver:
        return SERVICE_RECOGNIZER_DRIVER;
    case WinSystemService::ServiceType::kWin32OwnProcess:
        return SERVICE_WIN32_OWN_PROCESS;
    case WinSystemService::ServiceType::kWin32ShareProcess:
        return SERVICE_WIN32_SHARE_PROCESS;
    case WinSystemService::ServiceType::kUserService:
        return SERVICE_USER_SERVICE;
    case WinSystemService::ServiceType::kUserServiceInstance:
        return SERVICE_USERSERVICE_INSTANCE;
    case WinSystemService::ServiceType::kInteractiveProcess:
        return SERVICE_INTERACTIVE_PROCESS;
    case WinSystemService::ServiceType::kPkgService:
        return SERVICE_PKG_SERVICE;
    default:
        assert(false);                    // 未知的枚举值
        return SERVICE_WIN32_OWN_PROCESS; // 默认返回值
    }
}

WinSystemService::ServiceState CastServiceState(DWORD state)
{
    switch (state)
    {
    case SERVICE_STOPPED:
        return WinSystemService::ServiceState::kStoped;
    case SERVICE_RUNNING:
        return WinSystemService::ServiceState::kRunning;
    case SERVICE_PAUSED:
        return WinSystemService::ServiceState::kPaused;
    case SERVICE_START_PENDING:
        return WinSystemService::ServiceState::kStartPending;
    case SERVICE_STOP_PENDING:
        return WinSystemService::ServiceState::kStopPending;
    case SERVICE_CONTINUE_PENDING:
        return WinSystemService::ServiceState::kContinuePending;
    case SERVICE_PAUSE_PENDING:
        return WinSystemService::ServiceState::kPausePending;
    default:
        assert(false);                                  // 未知的服务状态
        return WinSystemService::ServiceState::kStoped; // 默认返回值
    }
}

DWORD CastServiceState(WinSystemService::ServiceState state)
{
    switch (state)
    {
    case WinSystemService::ServiceState::kStoped:
        return SERVICE_STOPPED;
    case WinSystemService::ServiceState::kRunning:
        return SERVICE_RUNNING;
    case WinSystemService::ServiceState::kPaused:
        return SERVICE_PAUSED;
    case WinSystemService::ServiceState::kStartPending:
        return SERVICE_START_PENDING;
    case WinSystemService::ServiceState::kStopPending:
        return SERVICE_STOP_PENDING;
    case WinSystemService::ServiceState::kContinuePending:
        return SERVICE_CONTINUE_PENDING;
    case WinSystemService::ServiceState::kPausePending:
        return SERVICE_PAUSE_PENDING;
    default:
        assert(false);          // 未知的枚举值
        return SERVICE_STOPPED; // 默认返回值
    }
}

zeus::expected<std::shared_ptr<QUERY_SERVICE_CONFIGW>, std::error_code> GetServiceConfig(const WinServiceHandle& handle)
{
    std::shared_ptr<QUERY_SERVICE_CONFIGW> config;
    DWORD                                  size = 0;
    do
    {
        if (QueryServiceConfigW(handle, config.get(), size, &size))
        {
            return config;
        }
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            config = std::shared_ptr<QUERY_SERVICE_CONFIGW>(
                reinterpret_cast<QUERY_SERVICE_CONFIGW*>(new uint8_t[size]), [](QUERY_SERVICE_CONFIGW* buffer) { delete[] buffer; }
            );
            continue;
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    while (true);
}

zeus::expected<std::unique_ptr<uint8_t[]>, std::error_code> GetServiceConfig2(const WinServiceHandle& handle, DWORD type)
{
    std::unique_ptr<uint8_t[]> config;
    DWORD                      size = 0;
    do
    {
        if (QueryServiceConfig2W(handle, type, config.get(), size, &size))
        {
            return config;
        }
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            config = std::make_unique<uint8_t[]>(size);
            continue;
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    while (true);
}
} // namespace

WinSysterServiceFailureAction::WinSysterServiceFailureAction() : _impl(std::make_unique<WinSysterServiceFailureActionImpl>())
{
}

WinSysterServiceFailureAction::WinSysterServiceFailureAction(WinSysterServiceFailureAction&& other) noexcept : _impl(std::move(other._impl))
{
}

WinSysterServiceFailureAction& WinSysterServiceFailureAction::operator=(WinSysterServiceFailureAction&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

WinSysterServiceFailureAction::~WinSysterServiceFailureAction()
{
}

std::optional<std::chrono::seconds> WinSysterServiceFailureAction::GetResetPeriod() const
{
    if (INFINITE == _impl->failAction.dwResetPeriod)
    {
        return std::nullopt;
    }
    else
    {
        return std::chrono::seconds(_impl->failAction.dwResetPeriod);
    }
}

std::string WinSysterServiceFailureAction::GetRebootMessage() const
{
    return CharsetUtils::UnicodeToUTF8(_impl->rebootMessage);
}

std::string WinSysterServiceFailureAction::GetCommand() const
{
    return CharsetUtils::UnicodeToUTF8(_impl->command);
}

std::vector<std::pair<WinSysterServiceFailureAction::FailureActionType, std::chrono::milliseconds>> WinSysterServiceFailureAction::GetActions() const
{
    std::vector<std::pair<FailureActionType, std::chrono::milliseconds>> actions;
    for (const auto& action : _impl->actions)
    {
        actions.emplace_back(std::make_pair(static_cast<FailureActionType>(action.Type), std::chrono::milliseconds(action.Delay)));
    }
    return actions;
}

void WinSysterServiceFailureAction::SetResetPeriod(const std::optional<std::chrono::seconds>& period)
{
    _impl->failAction.dwResetPeriod = period.has_value() ? static_cast<DWORD>(period->count()) : INFINITE;
}

void WinSysterServiceFailureAction::SetRebootMessage(const std::string& message)
{
    _impl->rebootMessage          = CharsetUtils::UTF8ToUnicode(message);
    _impl->failAction.lpRebootMsg = _impl->rebootMessage.data();
}

void WinSysterServiceFailureAction::SetCommand(const std::string& command)
{
    _impl->command              = CharsetUtils::UTF8ToUnicode(command);
    _impl->failAction.lpCommand = _impl->command.data();
}

void WinSysterServiceFailureAction::AddAction(FailureActionType action, const std::chrono::milliseconds& delay)
{
    SC_ACTION act = {};
    act.Type      = static_cast<SC_ACTION_TYPE>(action);
    act.Delay     = static_cast<DWORD>(delay.count());
    _impl->actions.emplace_back(std::move(act));
    _impl->failAction.cActions    = static_cast<DWORD>(_impl->actions.size());
    _impl->failAction.lpsaActions = _impl->actions.data();
}

void WinSysterServiceFailureAction::ClearActions()
{
    _impl->actions.clear();
    _impl->failAction.cActions    = 0;
    _impl->failAction.lpsaActions = _impl->actions.data();
}

WinSystemService::WinSystemService() : _impl(std::make_unique<WinSystemServiceImpl>())
{
}

WinSystemService::WinSystemService(WinSystemService&& other) noexcept : _impl(std::move(other._impl))
{
}

WinSystemService& WinSystemService::operator=(WinSystemService&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

WinSystemService::~WinSystemService()
{
}

std::string WinSystemService::Name() const
{
    return CharsetUtils::UnicodeToUTF8(_impl->serviceName);
}

zeus::expected<std::string, std::error_code> WinSystemService::GetDisplayName() const
{
    auto config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return CharsetUtils::UnicodeToUTF8(config->get()->lpDisplayName);
}

zeus::expected<std::string, std::error_code> WinSystemService::GetPath() const
{
    auto config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return CharsetUtils::UnicodeToUTF8(config->get()->lpBinaryPathName);
}

zeus::expected<WinSystemService::ServiceType, std::error_code> WinSystemService::GetServiceType() const
{
    auto config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return CastServiceType(config->get()->dwServiceType);
}

zeus::expected<WinSystemService::StartupType, std::error_code> WinSystemService::GetStartupType() const
{
    auto config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return CastStartupType(config->get()->dwStartType);
}

zeus::expected<WinSystemService::ServiceState, std::error_code> WinSystemService::GetServiceState() const
{
    SERVICE_STATUS serviceStatus = {};
    if (QueryServiceStatus(_impl->handle, &serviceStatus))
    {
        return CastServiceState(serviceStatus.dwCurrentState);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<std::string, std::error_code> WinSystemService::GetDescription() const
{
    auto config = GetServiceConfig2(_impl->handle, SERVICE_CONFIG_DESCRIPTION);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return CharsetUtils::UnicodeToUTF8(reinterpret_cast<SERVICE_DESCRIPTIONW*>(config->get())->lpDescription);
}

zeus::expected<bool, std::error_code> WinSystemService::IsDelayAutoStart() const
{
    auto config = GetServiceConfig2(_impl->handle, SERVICE_CONFIG_DELAYED_AUTO_START_INFO);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return TRUE == reinterpret_cast<SERVICE_DELAYED_AUTO_START_INFO*>(config->get())->fDelayedAutostart;
}

zeus::expected<std::vector<std::string>, std::error_code> WinSystemService::GetDependencies() const
{
    std::vector<std::string> dependencies;
    auto                     config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    if (config->get()->lpDependencies)
    {
        const wchar_t* dependency = config->get()->lpDependencies;
        while (*dependency)
        {
            dependencies.emplace_back(CharsetUtils::UnicodeToUTF8(dependency));
            dependency += dependencies.back().size() + 1;
        }
    }
    return dependencies;
}

zeus::expected<std::string, std::error_code> WinSystemService::GetGroup() const
{
    std::vector<std::string> dependencies;
    auto                     config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    if (config->get()->lpLoadOrderGroup)
    {
        return CharsetUtils::UnicodeToUTF8(config->get()->lpLoadOrderGroup);
    }
    return {};
}

zeus::expected<uint32_t, std::error_code> WinSystemService::GetGroupTag() const
{
    auto config = GetServiceConfig(_impl->handle);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    return config->get()->dwTagId;
}

zeus::expected<WinSysterServiceFailureAction, std::error_code> WinSystemService::GetFailureAction() const
{
    auto config = GetServiceConfig2(_impl->handle, SERVICE_CONFIG_FAILURE_ACTIONS);
    if (!config.has_value())
    {
        return zeus::unexpected(config.error());
    }
    const auto*                   failureActions = reinterpret_cast<SERVICE_FAILURE_ACTIONS*>(config->get());
    WinSysterServiceFailureAction action;
    action._impl->failAction.dwResetPeriod = failureActions->dwResetPeriod;
    if (failureActions->lpRebootMsg)
    {
        action._impl->rebootMessage = failureActions->lpRebootMsg;
    }
    action._impl->failAction.lpRebootMsg = action._impl->rebootMessage.data();
    if (failureActions->lpCommand)
    {
        action._impl->command = failureActions->lpCommand;
    }
    action._impl->failAction.lpCommand = action._impl->command.data();
    for (DWORD index = 0; index < failureActions->cActions; ++index)
    {
        action._impl->actions.emplace_back(failureActions->lpsaActions[index]);
    }
    action._impl->failAction.cActions    = failureActions->cActions;
    action._impl->failAction.lpsaActions = action._impl->actions.data();
    return std::move(action);
}

zeus::expected<void, std::error_code> WinSystemService::SetDisplayName(const std::string& displayName)
{
    if (ChangeServiceConfigW(
            _impl->handle, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            CharsetUtils::UTF8ToUnicode(displayName).c_str()
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetPath(const std::string& path)
{
    if (ChangeServiceConfigW(
            _impl->handle, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, CharsetUtils::UTF8ToUnicode(path).c_str(), nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetServiceType(ServiceType type)
{
    if (ChangeServiceConfigW(
            _impl->handle, CastServiceType(type), SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetStartupType(StartupType type)
{
    if (ChangeServiceConfigW(
            _impl->handle, SERVICE_NO_CHANGE, CastStartupType(type), SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetDescription(const std::string& description)
{
    std::wstring         data   = CharsetUtils::UTF8ToUnicode(description);
    SERVICE_DESCRIPTIONW buffer = {};
    buffer.lpDescription        = const_cast<wchar_t*>(data.c_str()); //服务描述信息
    if (ChangeServiceConfig2W(_impl->handle, SERVICE_CONFIG_DESCRIPTION, &buffer))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetDelayAutoStart(bool delay)
{
    SERVICE_DELAYED_AUTO_START_INFO info = {};
    info.fDelayedAutostart               = delay;
    if (ChangeServiceConfig2W(_impl->handle, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetDependencies(const std::vector<std::string>& dependencies)
{
    std::unique_ptr<wchar_t[]> dependenciesBuffer;
    size_t                     bufferSize = 10;
    std::vector<std::wstring>  dependenciesHolder;
    dependenciesHolder.reserve(dependencies.size());
    for (const auto& dependency : dependencies)
    {
        dependenciesHolder.emplace_back(CharsetUtils::UTF8ToUnicode(dependency));
        bufferSize += dependenciesHolder.size() + 1;
    }
    if (bufferSize)
    {
        dependenciesBuffer = std::make_unique<wchar_t[]>(bufferSize);
        std::memset(dependenciesBuffer.get(), 0, bufferSize);
        auto* data = dependenciesBuffer.get();
        for (const auto& dependency : dependenciesHolder)
        {
            std::memcpy(data, dependency.data(), dependency.size() * sizeof(wchar_t));
            data += dependency.size() + 1;
        }
    }
    if (ChangeServiceConfigW(
            _impl->handle, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, dependenciesBuffer.get(), nullptr,
            nullptr, nullptr
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetGroup(const std::string& group)
{
    if (ChangeServiceConfigW(
            _impl->handle, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, nullptr, CharsetUtils::UTF8ToUnicode(group).c_str(), nullptr,
            nullptr, nullptr, nullptr, nullptr
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetGroupTag(uint32_t tag)
{
    DWORD tagId = tag;
    if (ChangeServiceConfigW(
            _impl->handle, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, nullptr, nullptr, &tagId, nullptr, nullptr, nullptr, nullptr
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetFailureAction(const WinSysterServiceFailureAction& action)
{
    if (ChangeServiceConfig2W(_impl->handle, SERVICE_CONFIG_FAILURE_ACTIONS, &action._impl->failAction))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::SetBaseConfig(
    ServiceType serviceType, StartupType startType, const std::string& path, const std::string& displayName
)
{
    if (ChangeServiceConfigW(
            _impl->handle, CastServiceType(serviceType), CastStartupType(startType), SERVICE_NO_CHANGE, CharsetUtils::UTF8ToUnicode(path).c_str(),
            nullptr, nullptr, nullptr, nullptr, nullptr, CharsetUtils::UTF8ToUnicode(displayName).c_str()
        ))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<void, std::error_code> WinSystemService::Start(const std::vector<std::string>& params)
{
    bool ret = false;
    if (!params.empty())
    {
        std::vector<std::wstring> paramList;
        paramList.emplace_back(_impl->serviceName);
        for (const auto& param : params)
        {
            paramList.emplace_back(zeus::CharsetUtils::UTF8ToUnicode(param));
        }
        std::vector<const wchar_t*> serviceArgument(params.size() + 1);
        for (size_t i = 0; i < paramList.size(); i++)
        {
            serviceArgument[i] = paramList[i].c_str();
        }
        ret = StartServiceW(_impl->handle, serviceArgument.size(), serviceArgument.data());
    }
    else
    {
        ret = StartServiceW(_impl->handle, 0, nullptr);
    }
    if (!ret)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

zeus::expected<void, std::error_code> WinSystemService::Stop()
{
    SERVICE_STATUS serviceStatus {};
    const bool     ret = ControlService(_impl->handle, SERVICE_CONTROL_STOP, &serviceStatus);
    if (!ret)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

zeus::expected<void, std::error_code> WinSystemService::Remove()
{
    if (DeleteService(_impl->handle))
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<WinSystemService, std::error_code> WinSystemService::Open(const std::string& name, bool control, bool change)
{
    WinServiceHandle manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE));
    if (manager.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    DWORD access = SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG;
    if (control)
    {
        access |= (SERVICE_START | SERVICE_STOP | SERVICE_PAUSE_CONTINUE);
    }
    if (change)
    {
        access |= SERVICE_CHANGE_CONFIG | DELETE;
    }
    auto             wname = CharsetUtils::UTF8ToUnicode(name);
    WinServiceHandle service(OpenServiceW(manager, wname.c_str(), access));
    if (service.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WinSystemService systemService;
    systemService._impl->handle      = std::move(service);
    systemService._impl->serviceName = std::move(wname);
    return systemService;
}

zeus::expected<WinSystemService, std::error_code> WinSystemService::Create(
    const std::string& name, ServiceType serviceType, StartupType startType, const std::string& path, const std::string& displayName
)
{
    WinServiceHandle manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | SC_MANAGER_ENUMERATE_SERVICE));
    if (manager.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    auto             wname = CharsetUtils::UTF8ToUnicode(name);
    WinServiceHandle service(CreateServiceW(
        manager, wname.c_str(), CharsetUtils::UTF8ToUnicode(displayName).c_str(), SERVICE_ALL_ACCESS, CastServiceType(serviceType),
        CastStartupType(startType), SERVICE_ERROR_NORMAL, CharsetUtils::UTF8ToUnicode(path).c_str(), nullptr, nullptr, nullptr, nullptr, nullptr
    ));
    if (service.Empty())
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WinSystemService systemService;
    systemService._impl->handle      = std::move(service);
    systemService._impl->serviceName = std::move(wname);
    return systemService;
}

} // namespace zeus

#endif