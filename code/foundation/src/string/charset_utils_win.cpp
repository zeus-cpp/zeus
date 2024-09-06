#include "zeus/foundation/string/charset_utils.h"
#ifdef _WIN32
#include <Windows.h>

namespace zeus
{
std::string CharsetUtils::UnicodeToANSI(std::wstring_view source)
{
    std::string result;
    int         length = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, source.data(), source.size(), nullptr, 0, nullptr, FALSE);
    if (length)
    {
        result.resize(length);
        ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, source.data(), source.size(), result.data(), length, nullptr, FALSE);
    }
    return result;
}

std::wstring CharsetUtils::ANSIToUnicode(std::string_view source)
{
    std::wstring result;
    int          length = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source.data(), source.size(), NULL, 0);
    if (length)
    {
        result.resize(length);
        ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source.data(), source.size(), result.data(), length);
    }
    return result;
}

std::string CharsetUtils::UTF8ToANSI(std::string_view data)
{
    return UnicodeToANSI(UTF8ToUnicode(data));
}
std::string CharsetUtils::ANSIToUTF8(std::string_view data)
{
    return UnicodeToUTF8(ANSIToUnicode(data));
}

} // namespace zeus
#endif