#pragma once

#include "base_digest_impl.h"

namespace zeus
{
class SHA256DigestImpl :public BaseDigestImpl
{
public:
    /* Default construct. */
    SHA256DigestImpl();
};
}
