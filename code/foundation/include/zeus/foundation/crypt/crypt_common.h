#pragma once

namespace zeus
{

enum class AESMode
{
    ECB = 0,
    CBC = 1,
    CTR = 2,
    CFB = 3,
    OFB = 4,
};

enum class AESPadding
{
    NONE  = 0,
    PKCS7 = 1,
    ANSI_X_923,
    ISO_10126,
};

enum class HMACDigestType
{
    MD5  = 100,
    SHA1 = 200,
    SHA224,
    SHA256,
    SHA384,
    SHA512,
};

}
#include "zeus/foundation/core/zeus_compatible.h"
