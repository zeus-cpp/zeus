#pragma once
#include <string>
#include "zeus/foundation/crypt/crypt_common.h"

namespace zeus
{
inline std::string AESModeName(AESMode mode)
{
    switch (mode)
    {
    case zeus::AESMode::ECB:
        return "ECB";
        break;
    case zeus::AESMode::CBC:
        return "CBC";
        break;
    case zeus::AESMode::CTR:
        return "CTR";
        break;
    case zeus::AESMode::CFB:
        return "CFB";
        break;
    case zeus::AESMode::OFB:
        return "OFB";
        break;
    default:
        return "Unknown";
        break;
    }
}

inline std::string AESPaddingName(AESPadding padding)
{
    switch (padding)
    {
    case zeus::AESPadding::NONE:
        return "NONE";
        break;
    case zeus::AESPadding::PKCS7:
        return "PKCS7";
        break;
    case zeus::AESPadding::ANSI_X_923:
        return "ANSI_X_923";
        break;
    case zeus::AESPadding::ISO_10126:
        return "ISO_10126";
        break;
    default:
        return "Unknown";
        break;
    }
}
} // namespace zeus