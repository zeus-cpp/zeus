#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"
#include "zeus/foundation/crypt/crypt_common.h"

namespace zeus
{
class HMACDigestImpl;
class HMACDigest : public BaseDigest
{

public:
    HMACDigest(const void *saltByte, size_t saltLength, HMACDigestType type = HMACDigestType::SHA256);
    HMACDigest(const void *input, size_t length, const void *saltByte, size_t saltLength, HMACDigestType type = HMACDigestType::SHA256);
    HMACDigest(const std::string &str, const void *saltByte, size_t saltLength, HMACDigestType type = HMACDigestType::SHA256);
    HMACDigest(std::ifstream &in, const void *saltByte, size_t saltLength, HMACDigestType type = HMACDigestType::SHA256);
    HMACDigest(const std::filesystem::path &path, bool binary, const void *saltByte, size_t saltLength, HMACDigestType type = HMACDigestType::SHA256);
    ~HMACDigest() override;
    HMACDigest(const HMACDigest &)            = delete;
    HMACDigest &operator=(const HMACDigest &) = delete;
    HMACDigest(HMACDigest &&other) noexcept;
    HMACDigest      &operator=(HMACDigest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;

private:
    std::unique_ptr<HMACDigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
