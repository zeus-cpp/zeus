#include "md5_digest_impl.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

namespace zeus
{
Md5DigestImpl::Md5DigestImpl()
    :BaseDigestImpl(std::make_shared<CryptoPP::Weak::MD5>())
{
}
}
