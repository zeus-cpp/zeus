// Package: Utils

#include "zeus/foundation/string/charset_utils.h"
#include <string>
#include <cctype>
#include <cassert>
#include <codecvt>
#include <locale>
#include <cwchar>
#include <memory>
#include "zeus/foundation/byte/byte_utils.h"

using namespace std;

namespace zeus
{

optional<size_t> CharsetUtils::UTF8CharPrintCount(std::string_view data)
{
    if (data.empty())
    {
        return 0;
    }
    size_t totalCount = 0;
    for (size_t offset = 0; offset < data.size();)
    {
        auto *begin = data.data() + offset;
        if (*begin)
        {
            const auto count = CountLeftOne(*reinterpret_cast<const uint8_t *>(begin));
            if (0 == count)
            {
                ++totalCount;
                offset += 1;
            }
            else if (count >= 2)
            {
                ++totalCount;
                offset += count;
            }
            else
            {
                return nullopt;
            }
        }
        else
        {
            ++offset;
        }
    }
    return totalCount;
}

std::string CharsetUtils::UnicodeToUTF8(std::wstring_view source)
{
    wstring_convert<codecvt_utf8<wchar_t>> cv("");
    return cv.to_bytes(source.data(), source.data() + source.size());
}

std::wstring CharsetUtils::UTF8ToUnicode(std::string_view source)
{
    wstring_convert<codecvt_utf8<wchar_t>> cv("");
    return cv.from_bytes(source.data(), source.data() + source.size());
}

} // namespace zeus
