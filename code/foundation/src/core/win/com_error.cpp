#include "zeus/foundation/core/win/com_error.h"

#ifdef _WIN32

#include <comdef.h>
#include "zeus/foundation/string/charset_utils.h"

namespace zeus
{
const char* WinComCategory::name() const noexcept
{
    return "windows_com";
}
std::string WinComCategory::message(int hr) const
{
    return ComErrorMessage(hr);
}
std::error_condition WinComCategory::default_error_condition(int hr) const noexcept
{
    if (HRESULT_CODE(hr) || hr == 0)
    {
        // system error condition
        return std::system_category().default_error_condition(HRESULT_CODE(hr));
    }
    else
    {
        // special error condition
        return {hr, WinComCategory()};
    }
}
const std::error_category& WinComCategory::ErrorCategory()
{
    static WinComCategory category;
    return category;
}
std::string ComErrorMessage(HRESULT hr)
{
#ifdef _UNICODE
    return zeus::CharsetUtils::UnicodeToUTF8(_com_error {hr}.ErrorMessage());
#else
    return _com_error {hr}.ErrorMessage();
#endif
}
} // namespace zeus

#endif