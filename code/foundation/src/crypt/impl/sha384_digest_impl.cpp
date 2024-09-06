#include "sha384_digest_impl.h"
#include <cryptopp/sha.h>

namespace zeus
{
SHA384DigestImpl::SHA384DigestImpl() : BaseDigestImpl(std::make_shared<CryptoPP::SHA384>())
{
}
}
