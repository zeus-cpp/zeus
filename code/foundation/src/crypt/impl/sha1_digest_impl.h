#pragma once

#include "base_digest_impl.h"

namespace zeus
{
class SHA1DigestImpl :public BaseDigestImpl
{
public:
    /* Default construct. */
    SHA1DigestImpl();
};
}
