#include "crc32_digest_impl.h"

#include <cryptopp/crc.h>

namespace zeus
{
Crc32DigestImpl::Crc32DigestImpl()
    :BaseDigestImpl(std::make_shared<CryptoPP::CRC32>())
{
}
}
