#include "zeus/foundation/system/environment_variable.h"
#ifdef _WIN32
#include <memory>
#include <cstring>
#include <Windows.h>
#include <Userenv.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/security/win/token.h"

#pragma comment(lib, "Userenv.lib")

namespace zeus::EnvironmentVariable
{

std::optional<std::string> Get(std::string_view name)
{
    std::wstring uname = CharsetUtils::UTF8ToUnicode(name);
    DWORD        len   = GetEnvironmentVariableW(uname.c_str(), nullptr, 0);
    if (len == 0)
    {
        return std::nullopt;
    }
    len++;
    auto buffer = std::make_unique<wchar_t[]>(len);
    std::memset(buffer.get(), 0, sizeof(wchar_t) * len);
    GetEnvironmentVariableW(uname.c_str(), buffer.get(), len);
    return CharsetUtils::UnicodeToUTF8(buffer.get());
}

bool Has(std::string_view name)
{
    std::wstring uname = CharsetUtils::UTF8ToUnicode(name);
    const DWORD  len   = GetEnvironmentVariableW(uname.c_str(), nullptr, 0);
    return len > 0;
}

zeus::expected<void, std::error_code> Set(std::string_view name, std::string_view value)
{
    if (SetEnvironmentVariableW(CharsetUtils::UTF8ToUnicode(name).c_str(), CharsetUtils::UTF8ToUnicode(value).c_str()))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

zeus::expected<void, std::error_code> Unset(std::string_view name)
{
    if (SetEnvironmentVariableW(CharsetUtils::UTF8ToUnicode(name).c_str(), nullptr))
    {
        return {};
    }
    return zeus::unexpected(GetLastSystemError());
}

std::string ExpandEnvironmentVariableString(std::string_view data)
{
    const auto envParse = [](std::string_view key)
    {
        auto value = Get(key);
        if (value.has_value())
        {
            return std::move(value.value());
        }
        else
        {
            return std::string("%").append(key).append("%");
        }
    };
    return ExpandVariableString(data, envParse, "%", "%");
}

std::optional<std::string> GetReal(std::string_view name, const WinToken& token)
{
    constexpr DWORD len    = 1024;
    std::wstring    uname  = L"%" + CharsetUtils::UTF8ToUnicode(name) + L"%";
    auto            buffer = std::make_unique<wchar_t[]>(len);
    memset(buffer.get(), 0, sizeof(wchar_t) * len);
    if (ExpandEnvironmentStringsForUserW(token, uname.c_str(), buffer.get(), len))
    {
        if (uname != buffer.get())
        {
            return CharsetUtils::UnicodeToUTF8(buffer.get());
        }
    };
    return std::nullopt;
}
std::string ExpandEnvironmentVariableString(std::string_view data, const WinToken& token)
{
    const auto envParse = [&token](std::string_view key)
    {
        auto value = GetReal(key, token);
        if (value.has_value())
        {
            return std::move(value.value());
        }
        else
        {
            return std::string("%").append(key).append("%");
        }
    };
    return ExpandVariableString(data, envParse, "%", "%");
}
} // namespace zeus::EnvironmentVariable

#endif