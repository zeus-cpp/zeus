#include "sha224_digest_impl.h"
#include <cryptopp/sha.h>

namespace zeus
{
SHA224DigestImpl::SHA224DigestImpl() : BaseDigestImpl(std::make_shared<CryptoPP::SHA224>())
{
}
}
