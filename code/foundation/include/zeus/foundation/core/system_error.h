#pragma once

#if _WIN32
#include <Windows.h>
#else
#include <cerrno>
#endif

#include <cstdint>

#include <type_traits>
#include <system_error>

namespace zeus
{

#if defined(_WIN32)
// wrapper for DWORD
enum class SystemError : uint32_t
{
};
#else
enum class SystemError : int
{
};
#endif

}

namespace std
{

template<>
struct is_error_code_enum<zeus::SystemError> : true_type
{
};

}

namespace zeus
{

const std::error_category& SystemCategory() noexcept;

inline std::error_code make_error_code(SystemError e) noexcept
{
    return {static_cast<int>(e), SystemCategory()};
}

inline std::error_code GetLastSystemError() noexcept
{
#if _WIN32
    return make_error_code(SystemError {::GetLastError()});
#else
    return make_error_code(SystemError {errno});
#endif
}

inline std::error_code TranslateToSystemError(int code) noexcept
{
    return make_error_code(static_cast<SystemError>(code));
}

inline std::error_code TranslateToSystemError(std::error_code const& code) noexcept
{
    return TranslateToSystemError(code.value());
}

std::string SystemErrorMessage(int code);

inline std::string SystemErrorMessage(std::error_code const& code)
{
    return SystemErrorMessage(code.value());
}

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
