// Package: Utils

#include "zeus/foundation/string/string_utils.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cassert>
#include <fmt/format.h>
#include "zeus/foundation/byte/byte_utils.h"

namespace zeus
{

bool IsNumber(std::string_view str)
{
    return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
}

bool IsNumber(std::wstring_view str)
{
    return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
}

std::string_view TrimBeginView(std::string_view str)
{
    auto iter = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
    return str.substr(std::distance(str.begin(), iter));
}
std::string_view TrimEndView(std::string_view str)
{
    auto iter = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); });
    return str.substr(0, std::distance(iter, str.rend()));
}
std::string_view TrimView(std::string_view str)
{
    auto begin = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto end   = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); });
    return str.substr(std::distance(str.begin(), begin), std::distance(begin, end.base()));
}

std::wstring_view TrimBeginView(std::wstring_view str)
{
    auto iter = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
    return str.substr(std::distance(str.begin(), iter));
}
std::wstring_view TrimEndView(std::wstring_view str)
{
    auto iter = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); });
    return str.substr(0, std::distance(iter, str.rend()));
}
std::wstring_view TrimView(std::wstring_view str)
{
    auto begin = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto end   = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); });
    return str.substr(std::distance(str.begin(), begin), std::distance(begin, end.base()));
}

std::string TrimBegin(std::string_view str)
{
    return std::string(TrimBeginView(str));
}

std::string TrimEnd(std::string_view str)
{
    return std::string(TrimEndView(str));
}

std::string Trim(std::string_view str)
{
    return std::string(TrimView(str));
}

std::wstring TrimBegin(std::wstring_view str)
{
    return std::wstring(TrimBeginView(str));
}

std::wstring TrimEnd(std::wstring_view str)
{
    return std::wstring(TrimEndView(str));
}

std::wstring Trim(std::wstring_view str)
{
    return std::wstring(TrimView(str));
}

int Icompare(std::string_view str1, std::string_view str2)
{
    auto begin1(str1.begin());
    auto end1(str1.end());
    auto begin2(str2.begin());
    auto end2(str2.end());
    while (begin1 != end1 && begin2 != end2)
    {
        auto c1 = std::tolower(*begin1);
        auto c2 = std::tolower(*begin2);
        if (c1 < c2)
        {
            return -1;
        }
        else if (c1 > c2)
        {
            return 1;
        }
        ++begin1;
        ++begin2;
    }

    if (begin1 == end1)
    {
        return begin2 == end2 ? 0 : -1;
    }
    else
    {
        return 1;
    }
}

int Icompare(std::wstring_view str1, std::wstring_view str2)
{
    auto begin1(str1.begin());
    auto end1(str1.end());
    auto begin2(str2.begin());
    auto end2(str2.end());
    while (begin1 != end1 && begin2 != end2)
    {
        auto c1 = std::tolower(*begin1);
        auto c2 = std::tolower(*begin2);
        if (c1 < c2)
        {
            return -1;
        }
        else if (c1 > c2)
        {
            return 1;
        }
        ++begin1;
        ++begin2;
    }

    if (begin1 == end1)
    {
        return begin2 == end2 ? 0 : -1;
    }
    else
    {
        return 1;
    }
}

bool IEqual(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }
    return 0 == Icompare(str1, str2);
}
bool IEqual(std::wstring_view str1, std::wstring_view str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }
    return 0 == Icompare(str1, str2);
}
std::vector<std::string> Split(std::string_view str, std::string_view delim)
{
    std::vector<std::string> result;
    auto                     view = SplitView(str, delim);
    result.reserve(view.size());
    for (const auto item : view)
    {
        result.emplace_back(item);
    }
    return result;
}
std::vector<std::wstring> Split(std::wstring_view str, std::wstring_view delim)
{
    std::vector<std::wstring> result;
    auto                      view = SplitView(str, delim);
    result.reserve(view.size());
    for (const auto item : view)
    {
        result.emplace_back(item);
    }
    return result;
}

std::vector<std::string_view> SplitView(std::string_view str, std::string_view delim)
{
    std::vector<std::string_view> result;
    size_t                        offset     = 0;
    size_t                        findOffset = 0;
    while ((findOffset = str.find(delim, offset)) != std::string::npos)
    {
        if (findOffset != offset)
        {
            std::string_view item = str.substr(offset, findOffset - offset);
            result.emplace_back(std::move(item));
        }
        offset = findOffset + delim.size();
    }
    if (offset < str.size())
    {
        result.emplace_back(std::move(str.substr(offset, std::string::npos)));
    }
    return result;
}

std::vector<std::wstring_view> SplitView(std::wstring_view str, std::wstring_view delim)
{
    std::vector<std::wstring_view> result;
    size_t                         offset     = 0;
    size_t                         findOffset = 0;
    while ((findOffset = str.find(delim, offset)) != std::wstring::npos)
    {
        if (findOffset != offset)
        {
            std::wstring_view item = str.substr(offset, findOffset - offset);
            result.emplace_back(std::move(item));
        }
        offset = findOffset + delim.size();
    }
    if (offset < str.size())
    {
        result.emplace_back(std::move(str.substr(offset, std::wstring::npos)));
    }
    return result;
}

std::string Repeat(std::string_view str, size_t count)
{
    std::string result;
    result.reserve(str.size() * count);
    for (auto i = 0U; i < count; i++)
    {
        result.append(str);
    }
    return result;
}

std::wstring Repeat(std::wstring_view str, size_t count)
{
    std::wstring result;
    result.reserve(str.size() * count);
    for (auto i = 0U; i < count; i++)
    {
        result.append(str);
    }
    return result;
}

std::string Join(const std::vector<std::string>& strs, const std::string& delim)
{
    std::string result;
    if (strs.empty())
    {
        return result;
    }
    size_t size = strs.size() * delim.size();
    for (const auto& str : strs)
    {
        size += str.size();
    }
    result.reserve(size);
    for (size_t i = 0; i < strs.size(); i++)
    {
        if (i)
        {
            result.append(delim);
        }
        result.append(strs.at(i));
    }
    return result;
}

std::wstring Join(const std::vector<std::wstring>& strs, const std::wstring& delim)
{
    std::wstring result;
    if (strs.empty())
    {
        return result;
    }
    size_t size = strs.size() * delim.size();
    for (const auto& str : strs)
    {
        size += str.size();
    }
    result.reserve(size);
    for (size_t i = 0; i < strs.size(); i++)
    {
        if (i)
        {
            result.append(delim);
        }
        result.append(strs.at(i));
    }
    return result;
}

std::string JoinView(const std::vector<std::string_view>& strs, std::string_view delim)
{
    std::string result;
    if (strs.empty())
    {
        return result;
    }
    size_t size = strs.size() * delim.size();
    for (const auto str : strs)
    {
        size += str.size();
    }
    result.reserve(size);
    for (size_t i = 0; i < strs.size(); i++)
    {
        if (i)
        {
            result.append(delim);
        }
        result.append(strs.at(i));
    }
    return result;
}

std::wstring JoinView(const std::vector<std::wstring_view>& strs, std::wstring_view delim)
{
    std::wstring result;
    if (strs.empty())
    {
        return result;
    }
    size_t size = strs.size() * delim.size();
    for (const auto str : strs)
    {
        size += str.size();
    }
    result.reserve(size);
    for (size_t i = 0; i < strs.size(); i++)
    {
        if (i)
        {
            result.append(delim);
        }
        result.append(strs.at(i));
    }
    return result;
}

std::string Replace(std::string_view src, std::string_view substr, std::string_view replacement)
{
    std::string                  result(src);
    std::string::size_type       pos            = 0;
    const std::string::size_type srcLen         = substr.size();
    const std::string::size_type replacementLen = replacement.size();
    pos                                         = result.find(substr, pos);
    while ((pos != std::string::npos))
    {
        result.replace(pos, srcLen, replacement);
        pos = result.find(substr, (pos + replacementLen));
    }
    return result;
}

std::wstring Replace(std::wstring_view src, std::wstring_view substr, std::wstring_view replacement)
{
    std::wstring                  result(src);
    std::wstring::size_type       pos            = 0;
    const std::wstring::size_type srcLen         = substr.size();
    const std::wstring::size_type replacementLen = replacement.size();
    pos                                          = result.find(substr, pos);
    while ((pos != std::wstring::npos))
    {
        result.replace(pos, srcLen, replacement);
        pos = result.find(substr, (pos + replacementLen));
    }
    return result;
}

bool EndWith(std::string_view str, std::string_view end)
{
    if (end.empty())
    {
        return true;
    }
    if (end.size() > str.size())
    {
        return false;
    }
    auto size = std::min(str.size(), end.size());
    return 0 == std::memcmp(str.data() + str.size() - size, end.data(), size);
}
bool EndWith(std::wstring_view str, std::wstring_view end)
{
    if (end.empty())
    {
        return true;
    }
    if (end.size() > str.size())
    {
        return false;
    }
    auto size = std::min(str.size(), end.size());
    return 0 == std::memcmp(str.data() + str.size() - size, end.data(), size * sizeof(wchar_t));
}
bool StartWith(std::string_view str, std::string_view start)
{
    if (start.empty())
    {
        return true;
    }
    if (start.size() > str.size())
    {
        return false;
    }
    auto size = std::min(str.size(), start.size());
    return 0 == std::memcmp(str.data(), start.data(), size);
}
bool StartWith(std::wstring_view str, std::wstring_view start)
{
    if (start.empty())
    {
        return true;
    }
    if (start.size() > str.size())
    {
        return false;
    }
    auto size = std::min(str.size(), start.size());
    return 0 == std::memcmp(str.data(), start.data(), size * sizeof(wchar_t));
}
std::string ToLowerCopy(std::string_view str)
{
    std::string result(str);
    std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}
std::wstring ToLowerCopy(std::wstring_view str)
{
    std::wstring result(str);
    std::transform(str.begin(), str.end(), result.begin(), [](wchar_t c) { return std::tolower(c); });
    return result;
}
std::string ToUpperCopy(std::string_view str)
{
    std::string result(str);
    std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
    return result;
}
std::wstring ToUpperCopy(std::wstring_view str)
{
    std::wstring result(str);
    std::transform(str.begin(), str.end(), result.begin(), [](wchar_t c) { return std::toupper(c); });
    return result;
}
void ToLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}
void ToLower(std::wstring& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](wchar_t c) { return std::tolower(c); });
}
void ToUpper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
}
void ToUpper(std::wstring& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](wchar_t c) { return std::toupper(c); });
}
std::string BytesToHexString(const void* input, size_t length, bool upCase)
{
    static const char UP_HEX[16]  = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static const char LOW_HEX[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    const char*       hex         = upCase ? UP_HEX : LOW_HEX;

    std::string str;
    str.reserve(length << 1);
    for (size_t i = 0; i < length; i++)
    {
        int t = reinterpret_cast<const uint8_t*>(input)[i];
        int a = t / 16;
        int b = t % 16;
        str.append(1, hex[a]);
        str.append(1, hex[b]);
    }
    return str;
}
std::string IntToHexString(uint32_t integer, bool upCase)
{
    return fmt::format(upCase ? "{:08X}" : "{:08x}", integer);
}
std::string IntToHexString(int32_t integer, bool upCase)
{
    return fmt::format(upCase ? "{:08X}" : "{:08x}", integer);
}
std::string IntToHexString(uint64_t integer, bool upCase)
{
    return fmt::format(upCase ? "{:016X}" : "{:016x}", integer);
}
std::string IntToHexString(int64_t integer, bool upCase)
{
    return fmt::format(upCase ? "{:016X}" : "{:016x}", integer);
}

std::string ExpandVariableString(
    std::string_view data, const std::function<std::string(std::string_view)>& variableReplace, std::string_view prefix, std::string_view suffix
)
{
    assert(variableReplace);
    std::string newData(data);

    auto tagBeing = data.find(prefix);
    while (std::string::npos != tagBeing)
    {
        const auto tagEnd = data.find(suffix, tagBeing + 1);
        if (std::string::npos == tagBeing)
        {
            break;
        }

        std::string key(data.substr(tagBeing + 1, tagEnd - tagBeing - 1));

        std::string variable = std::string(prefix) + key + std::string(suffix);
        std::string value    = variableReplace(key);
        newData              = Replace(newData, variable, value);

        tagBeing = data.find(prefix, tagEnd + 1);
    }

    return newData;
}

std::vector<std::string> SplitMultiString(const void* data, size_t length)
{
    std::vector<std::string> result;
    auto split = ByteSplit({static_cast<const uint8_t*>(data), length}, {reinterpret_cast<const uint8_t*>("\0"), sizeof(char)}, sizeof(char));
    result.reserve(split.size());
    for (const auto& element : split)
    {
        if (element.Size())
        {
            result.emplace_back(reinterpret_cast<const char*>(element.Data()));
        }
    }
    return result;
}

std::vector<std::wstring> SplitMultiWString(const void* data, size_t length)
{
    std::vector<std::wstring> result;
    auto split = ByteSplit({static_cast<const uint8_t*>(data), length}, {reinterpret_cast<const uint8_t*>(L"\0"), sizeof(wchar_t)}, sizeof(wchar_t));
    result.reserve(split.size());
    for (const auto& element : split)
    {
        if (element.Size())
        {
            result.emplace_back(reinterpret_cast<const wchar_t*>(element.Data()));
        }
    }
    return result;
}

std::string Unquote(std::string_view str)
{
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
    {
        str.remove_prefix(1);
        str.remove_suffix(1);
    }
    return std::string(str);
}

bool IsASCIIAlphaNumeric(char character)
{
    if (character >= '0' && character <= '9')
    {
        return true;
    }
    if (character >= 'A' && character <= 'Z')
    {
        return true;
    }
    if (character >= 'a' && character <= 'z')
    {
        return true;
    }
    return false;
}
bool IsASCIIAlphaNumeric(wchar_t character)
{
    if (character >= L'0' && character <= L'9')
    {
        return true;
    }
    if (character >= L'A' && character <= L'Z')
    {
        return true;
    }
    if (character >= L'a' && character <= L'z')
    {
        return true;
    }
    return false;
}

} // namespace zeus
