#include "zeus/foundation/string/url_utils.h"
#include <sstream>
#include <iomanip>
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{
std::string UriEscape(std::string_view str)
{
    std::stringstream stream;
    stream.imbue(std::locale::classic());
    stream.fill('0');
    for (const char& item : str)
    {
        if (IsASCIIAlphaNumeric(item) || item == '-' || item == '_' || item == '.' || item == '~')
        {
            stream << item;
        }
        else
        {
            stream << std::uppercase;
            stream << '%' << std::setw(2) << std::hex << static_cast<uint32_t>(static_cast<uint8_t>(item));
            stream << std::nouppercase;
        }
    }
    return stream.str();
}
std::string UriUnescape(std::string_view str)
{
    std::string decoded;
    decoded.reserve(str.length()); // 预分配空间以提高性能

    for (auto iter = str.begin(); iter != str.end(); ++iter)
    {
        if (*iter == '%')
        {
            if (std::distance(iter, str.end()) < 3)
            {
                // URL 编码不完整，直接添加 '%'
                decoded += '%';
            }
            else
            {
                int                hex;
                std::string        hex_str(iter + 1, iter + 3);
                std::istringstream iss(hex_str);
                iss.imbue(std::locale::classic());
                if (iss >> std::hex >> hex)
                {
                    decoded += static_cast<char>(hex);
                    std::advance(iter, 2);
                }
                else
                {
                    // 无效的十六进制编码，保持原样
                    decoded += '%';
                }
            }
        }
        else if (*iter == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += *iter;
        }
    }

    return decoded;
}

std::string GenerateUrlQueryString(const std::unordered_map<std::string, std::string>& params)
{
    std::ostringstream stream;
    bool               empty = true;
    stream.imbue(std::locale::classic());
    for (const auto& [key, value] : params)
    {
        if (!empty)
        {
            stream << "&";
        }
        stream << UriEscape(key) << "=" << UriEscape(value);
        empty = false;
    }
    return stream.str();
}
std::string GenerateUrlQueryString(const std::map<std::string, std::string>& params)
{
    std::ostringstream stream;
    bool               empty = true;
    stream.imbue(std::locale::classic());
    for (const auto& [key, value] : params)
    {
        if (!empty)
        {
            stream << "&";
        }
        stream << UriEscape(key) << "=" << UriEscape(value);
        empty = false;
    }
    return stream.str();
}
std::string GenerateUrlQueryString(const std::unordered_map<std::string_view, std::string_view>& params)
{
    std::ostringstream stream;
    bool               empty = true;
    stream.imbue(std::locale::classic());
    for (const auto [key, value] : params)
    {
        if (!empty)
        {
            stream << "&";
        }
        stream << UriEscape(key) << "=" << UriEscape(value);
        empty = false;
    }
    return stream.str();
}
std::string GenerateUrlQueryString(const std::map<std::string_view, std::string_view>& params)
{
    std::ostringstream stream;
    bool               empty = true;
    stream.imbue(std::locale::classic());
    for (const auto [key, value] : params)
    {
        if (!empty)
        {
            stream << "&";
        }
        stream << UriEscape(key) << "=" << UriEscape(value);
        empty = false;
    }
    return stream.str();
}
} // namespace zeus