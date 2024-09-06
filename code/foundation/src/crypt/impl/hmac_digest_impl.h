#pragma once

#include "zeus/foundation/crypt/crypt_common.h"
#include "base_digest_impl.h"

namespace zeus
{
class HMACDigestImpl : public BaseDigestImpl
{
public:
    /* Default construct. */
    HMACDigestImpl(HMACDigestType type, const void* key, size_t length);
    std::string Name();
private:
    HMACDigestType _type;
};
}
