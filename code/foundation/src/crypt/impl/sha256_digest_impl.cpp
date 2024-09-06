#include "sha256_digest_impl.h"
#include <cryptopp/sha.h>

namespace zeus
{
SHA256DigestImpl::SHA256DigestImpl()
    :BaseDigestImpl(std::make_shared<CryptoPP::SHA256>())
{
}
}
