#pragma once

#include "base_digest_impl.h"

namespace zeus
{
class Crc32DigestImpl : public BaseDigestImpl
{
public:
    /* Default construct. */
    Crc32DigestImpl();
};
}
