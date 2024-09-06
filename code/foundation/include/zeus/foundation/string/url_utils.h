#pragma once
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
namespace zeus
{
std::string UriEscape(std::string_view str);
std::string UriUnescape(std::string_view str);

//结果不添加前置的?
std::string GenerateUrlQueryString(const std::unordered_map<std::string, std::string>& params);
std::string GenerateUrlQueryString(const std::map<std::string, std::string>& params);
std::string GenerateUrlQueryString(const std::unordered_map<std::string_view, std::string_view>& params);
std::string GenerateUrlQueryString(const std::map<std::string_view, std::string_view>& params);
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"