#include "hmac_digest_impl.h"
#include <cassert>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/hmac.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha.h>

namespace zeus
{
static std::shared_ptr<CryptoPP::HashTransformation> CreateTransform(HMACDigestType type, const uint8_t *key, size_t length)
{
    switch (type)
    {
    case HMACDigestType::MD5:
        return std::make_shared<CryptoPP::HMAC<CryptoPP::Weak::MD5>>(key, length);
        break;
    case HMACDigestType::SHA1:
        return std::make_shared<CryptoPP::HMAC<CryptoPP::SHA1>>(key, length);
        break;
    case HMACDigestType::SHA224:
        return std::make_shared<CryptoPP::HMAC<CryptoPP::SHA224>>(key, length);
        break;
    case HMACDigestType::SHA256:
        return std::make_shared<CryptoPP::HMAC<CryptoPP::SHA256>>(key, length);
        break;
    case HMACDigestType::SHA384:
        return std::make_shared<CryptoPP::HMAC<CryptoPP::SHA384>>(key, length);
        break;
    case HMACDigestType::SHA512:
        return std::make_shared<CryptoPP::HMAC<CryptoPP::SHA512>>(key, length);
        break;
    default:
        return nullptr;
    }
}

HMACDigestImpl::HMACDigestImpl(HMACDigestType type, const void *key, size_t length)
    : BaseDigestImpl(CreateTransform(type, reinterpret_cast<const uint8_t *>(key), length)), _type(type)
{
}
std::string HMACDigestImpl::Name()
{
    switch (_type)
    {
    case HMACDigestType::MD5:
        return "HMAC(MD5)";
    case HMACDigestType::SHA1:
        return "HMAC(SHA1)";
    case HMACDigestType::SHA224:
        return "HMAC(SHA224)";
    case HMACDigestType::SHA256:
        return "HMAC(SHA256)";
    case HMACDigestType::SHA384:
        return "HMAC(SHA384)";
    case HMACDigestType::SHA512:
        return "HMAC(SHA512)";
    default:
        assert(false);
        return "HMAC(Unknown)";
    }
}
} // namespace zeus
