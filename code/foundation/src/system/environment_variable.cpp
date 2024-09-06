#include "zeus/foundation/system/environment_variable.h"
#include <cassert>
#include <cstring>
#include <list>
#include <array>
#include <memory>
#include "zeus/foundation/byte/byte_utils.h"
#include "zeus/foundation/string/charset_utils.h"

namespace zeus::EnvironmentVariable
{
std::map<std::string, std::string> ParseEnvironmentVariableArray(char** data)
{
    std::map<std::string, std::string> result;
    for (char** item = data; *item; ++item)
    {
        auto delim = std::strchr(*item, '=');
        if (delim)
        {
            result.emplace(std::string(*item, delim), delim + 1);
        }
    }
    return result;
}

std::map<std::string, std::string> ParseEnvironmentVariableData(const void* data, size_t length)
{
    auto split = ByteSplit({static_cast<const uint8_t*>(data), length}, {reinterpret_cast<const uint8_t*>("\0"), sizeof(char)}, sizeof(char));
    std::map<std::string, std::string> result;
    for (const auto& item : split)
    {
        auto delim = ByteFind(item, {reinterpret_cast<const uint8_t*>("="), sizeof(char)}, sizeof(char));
        if (std::string::npos != delim)
        {
            std::string key(reinterpret_cast<const char*>(item.Data()), delim);
            std::string value(reinterpret_cast<const char*>(item.Data() + delim + sizeof(char)));
            result.emplace(std::move(key), std::move(value));
        }
    }
    return result;
}

std::map<std::wstring, std::wstring> ParseEnvironmentVariableDataW(const void* data, size_t length)
{
    auto split = ByteSplit({static_cast<const uint8_t*>(data), length}, {reinterpret_cast<const uint8_t*>(L"\0"), sizeof(wchar_t)}, sizeof(wchar_t));
    std::map<std::wstring, std::wstring> result;
    for (const auto& item : split)
    {
        auto delim = ByteFind(item, {reinterpret_cast<const uint8_t*>(L"="), sizeof(wchar_t)}, sizeof(wchar_t));
        if (std::string::npos != delim)
        {
            result.emplace(
                std::wstring(reinterpret_cast<const wchar_t*>(item.Data()), delim / sizeof(wchar_t)),
                std::wstring(reinterpret_cast<const wchar_t*>(item.Data() + delim + sizeof(wchar_t)))
            );
        }
    }
    return result;
}
std::map<std::string, std::string> ParseEnvironmentVariableData(const void* data)
{
    assert(data);
    std::array<uint8_t, sizeof(char) * 2> end    = {0};
    auto const*                           offset = static_cast<const uint8_t*>(data);
    while (0 != std::memcmp(end.data(), offset, sizeof(end)))
    {
        offset++;
    }
    return ParseEnvironmentVariableData(data, offset - static_cast<const uint8_t*>(data) + sizeof(uint8_t) * end.size());
}
std::map<std::wstring, std::wstring> ParseEnvironmentVariableDataW(const void* data)
{
    assert(data);
    std::array<uint8_t, sizeof(wchar_t) * 2> end    = {0};
    auto const*                              offset = static_cast<const uint8_t*>(data);
    while (0 != std::memcmp(end.data(), offset, sizeof(end)))
    {
        offset++;
    }
    return ParseEnvironmentVariableDataW(data, offset - static_cast<const uint8_t*>(data) + sizeof(uint8_t) * end.size());
}
std::vector<uint8_t> CreateEnvironmentVariableData(const std::map<std::string, std::string>& data)
{
    std::list<std::string> items;
    for (const auto& item : data)
    {
        items.emplace_back(item.first + "=" + item.second);
    }

    std::vector<ByteBufferView> byteItems;
    for (const auto& item : items)
    {
        byteItems.emplace_back(reinterpret_cast<const uint8_t*>(item.data()), item.size());
    }
    byteItems.emplace_back(reinterpret_cast<const uint8_t*>("\0"), sizeof(char));
    return ByteJoin(byteItems, {reinterpret_cast<const uint8_t*>("\0"), sizeof(char)});
}

std::vector<uint8_t> CreateEnvironmentVariableData(const std::map<std::wstring, std::wstring>& data)
{
    std::list<std::wstring> items;
    for (const auto& item : data)
    {
        items.emplace_back(item.first + L"=" + item.second);
    }

    std::vector<ByteBufferView> byteItems;
    for (const auto& item : items)
    {
        byteItems.emplace_back(reinterpret_cast<const uint8_t*>(item.data()), item.size() * sizeof(wchar_t));
    }
    byteItems.emplace_back(reinterpret_cast<const uint8_t*>(L"\0"), sizeof(wchar_t));
    return ByteJoin(byteItems, {reinterpret_cast<const uint8_t*>(L"\0"), sizeof(wchar_t)});
}
} // namespace zeus::EnvironmentVariable
