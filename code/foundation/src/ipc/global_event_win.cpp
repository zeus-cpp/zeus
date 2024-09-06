#include "zeus/foundation/ipc/global_event.h"
#ifdef _WIN32

#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/resource/win/handle.h"

namespace zeus
{
struct GlobalEventImpl
{
    std::string name;
    WinHandle   handle;
};
GlobalEvent::GlobalEvent() : _impl(std::make_unique<GlobalEventImpl>())
{
}

GlobalEvent::GlobalEvent(GlobalEvent&& other) noexcept : _impl(std::move(other._impl))
{
}

GlobalEvent& GlobalEvent::operator=(GlobalEvent&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

GlobalEvent::~GlobalEvent()
{
}

std::string GlobalEvent::Name() const
{
    return _impl->name;
}

zeus::expected<void, std::error_code> GlobalEvent::Set()
{
    if (SetEvent(_impl->handle))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

zeus::expected<void, std::error_code> GlobalEvent::Reset()
{
    if (ResetEvent(_impl->handle))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

zeus::expected<void, std::error_code> GlobalEvent::Wait()
{
    if (WaitForSingleObject(_impl->handle, INFINITE) == WAIT_OBJECT_0)
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

zeus::expected<bool, std::error_code> GlobalEvent::WaitTimeout(const std::chrono::steady_clock::duration& duration)
{
    const auto wait = WaitForSingleObject(_impl->handle, std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    switch (wait)
    {
    case WAIT_OBJECT_0:
        return true;
    case WAIT_TIMEOUT:
        return false;
    default:
        return zeus::unexpected(GetLastSystemError());
    }
}

zeus::expected<GlobalEvent, std::error_code> GlobalEvent::OpenOrCreate(const std::string& name, bool manual)
{
    SECURITY_ATTRIBUTES securittyAttr = {};
    securittyAttr.nLength             = sizeof(SECURITY_ATTRIBUTES);
    securittyAttr.bInheritHandle      = FALSE;
    SECURITY_DESCRIPTOR sd            = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    SetSecurityDescriptorGroup(&sd, nullptr, FALSE);
    SetSecurityDescriptorSacl(&sd, FALSE, nullptr, FALSE);
    securittyAttr.lpSecurityDescriptor = &sd;
    WinHandle handle                   = CreateEventW(&securittyAttr, manual, FALSE, CharsetUtils::UTF8ToUnicode(name).c_str());
    if (!handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    GlobalEvent event;
    event._impl->handle = std::move(handle);
    event._impl->name   = name;
    return std::move(event);
}

zeus::expected<GlobalEvent, std::error_code> GlobalEvent::Open(const std::string& name)
{
    WinHandle handle = OpenEventW(EVENT_MODIFY_STATE, FALSE, CharsetUtils::UTF8ToUnicode(name).c_str());
    if (!handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    GlobalEvent event;
    event._impl->handle = std::move(handle);
    event._impl->name   = name;
    return std::move(event);
}
} // namespace zeus
#endif