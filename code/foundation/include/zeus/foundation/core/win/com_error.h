#pragma once

#ifdef _WIN32

#include <system_error>
#include "zeus/foundation/core/win/win_windef.h"

namespace zeus
{

std::string ComErrorMessage(HRESULT hr);

class WinComCategory : public std::error_category
{

public:

    const char* name() const noexcept override;

    // @note If _UNICODE is defined the error description gets
    // converted to an ANSI string using the CP_ACP codepage.
    std::string message(int hr) const override;

    // Make error_condition for error code (generic if possible)
    // @return system's default error condition if error value can be mapped to a Windows error, error condition with com category otherwise
    std::error_condition default_error_condition(int hr) const noexcept override;

    static const std::error_category& ErrorCategory();
};
}

#endif
#include "zeus/foundation/core/zeus_compatible.h"
