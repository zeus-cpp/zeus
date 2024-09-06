#pragma once
#include <string>
#include <optional>
#include <string_view>
namespace zeus
{
class CharsetUtils
{
public:
    static std::optional<size_t> UTF8CharPrintCount(std::string_view data);
    static std::string           UnicodeToUTF8(std::wstring_view source);
    static std::wstring          UTF8ToUnicode(std::string_view source);
#ifdef _WIN32
    static std::string  UnicodeToANSI(std::wstring_view source);
    static std::wstring ANSIToUnicode(std::string_view source);
    static std::string  UTF8ToANSI(std::string_view data);
    static std::string  ANSIToUTF8(std::string_view data);
#endif
};
}

#include "zeus/foundation/core/zeus_compatible.h"
