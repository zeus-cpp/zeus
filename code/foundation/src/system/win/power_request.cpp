#include "zeus/foundation/system/win/power_request.h"

#ifdef _WIN32

#include <Windows.h>
#include <winbase.h>

#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/core/system_error.h"

namespace
{
POWER_REQUEST_TYPE ConvertPowerRequestType(zeus::WinPowerRequestType type)
{
    using namespace zeus;
    switch (type)
    {
    case WinPowerRequestType::DisplayRequried:
        return PowerRequestDisplayRequired;
    case WinPowerRequestType::SystemRequired:
        return PowerRequestSystemRequired;
    case WinPowerRequestType::AwayModeRequired:
        return PowerRequestAwayModeRequired;
    case WinPowerRequestType::ExecucutionRequired:
        return PowerRequestExecutionRequired;
    default:
        return PowerRequestDisplayRequired;
    }
}
} // namespace

namespace zeus
{

WinPowerRequest::~WinPowerRequest()
{
}

class WinPowerRequestImpl : public WinPowerRequest
{
public:
    WinPowerRequestImpl(zeus::WinHandle handle, WinPowerRequestType type, std::string const& reason) noexcept
        : _requestHandle(std::move(handle)), _type(type), _reason(reason)
    {
    }

    ~WinPowerRequestImpl() noexcept override
    {
        if (_requestHandle)
        {
            ::PowerClearRequest(_requestHandle, ConvertPowerRequestType(_type));
        }
    }

    virtual WinPowerRequestType Type() const { return _type; }

    virtual std::string Reason() const { return _reason; }

private:
    zeus::WinHandle     _requestHandle;
    WinPowerRequestType _type;
    std::string         _reason;
};

zeus::expected<std::unique_ptr<WinPowerRequest>, std::error_code> WinPowerRequestFactory::Create(WinPowerRequestType type, const std::string& reason)
{
    REASON_CONTEXT reasonContext;
    ZeroMemory(&reasonContext, sizeof(REASON_CONTEXT));
    reasonContext.Version                   = POWER_REQUEST_CONTEXT_VERSION;
    reasonContext.Flags                     = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
    std::wstring wReason                    = zeus::CharsetUtils::UTF8ToUnicode(reason);
    reasonContext.Reason.SimpleReasonString = &wReason[0];

    zeus::WinHandle handle = ::PowerCreateRequest(&reasonContext);
    if (handle != INVALID_HANDLE_VALUE)
    {
        if (::PowerSetRequest(handle, ConvertPowerRequestType(type)))
        {
            // NOTE: 这个 handle 不能被 duplicate，故 move 了 zeus::AutoHandle
            return std::make_unique<WinPowerRequestImpl>(std::move(handle), type, reason);
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}

} // namespace zeus

#endif
