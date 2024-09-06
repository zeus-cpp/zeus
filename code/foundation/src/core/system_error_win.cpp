#include "zeus/foundation/core/system_error.h"

#ifdef _WIN32
#include <memory>
#include <Windows.h>
#include "zeus/foundation/string/charset_utils.h"

namespace zeus
{

std::string SystemErrorMessage(int code)
{
    uint32_t                   bufferLen = 64;
    size_t                     length    = 0;
    std::unique_ptr<wchar_t[]> buffer    = std::make_unique<wchar_t[]>(bufferLen);
    for (;;)
    {
        length = ::FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, 0, code,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), buffer.get(), bufferLen, 0
        );
        if (0 == length && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            bufferLen *= 2;
            buffer = std::make_unique<wchar_t[]>(bufferLen);
            continue;
        }
        break;
    }
    return CharsetUtils::UnicodeToUTF8(std::wstring(buffer.get(), length));
}

} // namespace zeus

#endif