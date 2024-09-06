#pragma once

#include "base_digest_impl.h"

namespace zeus
{
class Md5DigestImpl :public BaseDigestImpl
{
public:
    /* Default construct. */
    Md5DigestImpl();
};
}
