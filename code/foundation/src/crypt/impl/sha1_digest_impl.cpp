#include "sha1_digest_impl.h"
#include <cryptopp/sha.h>

namespace zeus
{
SHA1DigestImpl::SHA1DigestImpl()
    :BaseDigestImpl(std::make_shared<CryptoPP::SHA1>())
{
}
}
