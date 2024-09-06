#pragma once

#include <cassert>
#include <cctype>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <string_view>

namespace zeus
{

bool              IsNumber(std::string_view str);
bool              IsNumber(std::wstring_view str);
std::string_view  TrimBeginView(std::string_view str);
std::string_view  TrimEndView(std::string_view str);
std::string_view  TrimView(std::string_view str);
std::wstring_view TrimBeginView(std::wstring_view str);
std::wstring_view TrimEndView(std::wstring_view str);
std::wstring_view TrimView(std::wstring_view str);

std::string  TrimBegin(std::string_view str);
std::string  TrimEnd(std::string_view str);
std::string  Trim(std::string_view str);
std::wstring TrimBegin(std::wstring_view str);
std::wstring TrimEnd(std::wstring_view str);
std::wstring Trim(std::wstring_view str);

int  Icompare(std::string_view str1, std::string_view str2);
int  Icompare(std::wstring_view str1, std::wstring_view str2);
bool IEqual(std::string_view str1, std::string_view str2);
bool IEqual(std::wstring_view str1, std::wstring_view str2);

std::vector<std::string>  Split(std::string_view str, std::string_view delim);
std::vector<std::wstring> Split(std::wstring_view str, std::wstring_view delim);

std::vector<std::string_view>  SplitView(std::string_view str, std::string_view delim);
std::vector<std::wstring_view> SplitView(std::wstring_view str, std::wstring_view delim);

std::string  Repeat(std::string_view str, size_t count);
std::wstring Repeat(std::wstring_view str, size_t count);

std::string  Join(const std::vector<std::string>& strs, const std::string& delim);
std::wstring Join(const std::vector<std::wstring>& strs, const std::wstring& delim);

std::string  JoinView(const std::vector<std::string_view>& strs, std::string_view delim);
std::wstring JoinView(const std::vector<std::wstring_view>& strs, std::wstring_view delim);

std::string  Replace(std::string_view src, std::string_view substr, std::string_view replacement);
std::wstring Replace(std::wstring_view src, std::wstring_view substr, std::wstring_view replacement);

bool EndWith(std::string_view str, std::string_view end);
bool EndWith(std::wstring_view str, std::wstring_view end);

bool StartWith(std::string_view str, std::string_view start);
bool StartWith(std::wstring_view str, std::wstring_view start);

std::string  ToLowerCopy(std::string_view str);
std::wstring ToLowerCopy(std::wstring_view str);
std::string  ToUpperCopy(std::string_view str);
std::wstring ToUpperCopy(std::wstring_view str);

void ToLower(std::string& str);
void ToLower(std::wstring& str);
void ToUpper(std::string& str);
void ToUpper(std::wstring& str);

std::string BytesToHexString(const void* input, size_t length, bool upCase);

std::string IntToHexString(uint32_t integer, bool upCase = true);
std::string IntToHexString(int32_t integer, bool upCase = true);
std::string IntToHexString(uint64_t integer, bool upCase = true);
std::string IntToHexString(int64_t integer, bool upCase = true);

//展开含有变量的字符串
std::string ExpandVariableString(
    std::string_view data, const std::function<std::string(std::string_view)>& variableReplace, std::string_view prefix, std::string_view suffix
);

std::vector<std::string>  SplitMultiString(const void* data, size_t length);
std::vector<std::wstring> SplitMultiWString(const void* data, size_t length);

std::string Unquote(std::string_view str);

bool IsASCIIAlphaNumeric(char character);
bool IsASCIIAlphaNumeric(wchar_t character);

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
