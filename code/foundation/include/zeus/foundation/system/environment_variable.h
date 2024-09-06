#pragma once

#include <cstdint>

#include <map>
#include <vector>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{
#ifdef _WIN32
class WinToken;
#endif
namespace EnvironmentVariable
{
std::optional<std::string>            Get(std::string_view name);
bool                                  Has(std::string_view name);
zeus::expected<void, std::error_code> Set(std::string_view name, std::string_view value);
zeus::expected<void, std::error_code> Unset(std::string_view name);
std::map<std::string, std::string>    ParseEnvironmentVariableArray(char** data);
std::map<std::string, std::string>    ParseEnvironmentVariableData(const void* data, size_t length);
std::map<std::wstring, std::wstring>  ParseEnvironmentVariableDataW(const void* data, size_t length);
std::map<std::string, std::string>    ParseEnvironmentVariableData(const void* data);
std::map<std::wstring, std::wstring>  ParseEnvironmentVariableDataW(const void* data);
std::vector<uint8_t>                  CreateEnvironmentVariableData(const std::map<std::string, std::string>& data);
std::vector<uint8_t>                  CreateEnvironmentVariableData(const std::map<std::wstring, std::wstring>& data);
//展开含有环境变量的字符串 windows:%var% linux:${var}
std::string                           ExpandEnvironmentVariableString(std::string_view data);

#ifdef _WIN32
//使用用户Token获取指定用户实时的环境变量
std::optional<std::string> GetReal(std::string_view name, const WinToken& token);

//使用用户Token展开含有环境变量的字符串
std::string ExpandEnvironmentVariableString(std::string_view data, const WinToken& token);

#endif
}
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
