#include "sha512_digest_impl.h"
#include <cryptopp/sha.h>

namespace zeus
{
SHA512DigestImpl::SHA512DigestImpl() : BaseDigestImpl(std::make_shared<CryptoPP::SHA512>())
{
}
}
