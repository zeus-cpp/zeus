#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class SHA1DigestImpl;
class SHA1Digest : public BaseDigest
{
public:
    SHA1Digest();
    SHA1Digest(const void *input, size_t length);
    SHA1Digest(const std::string &str);
    SHA1Digest(std::ifstream &in);
    SHA1Digest(const std::filesystem::path &path, bool binary);
    ~SHA1Digest() override;
    SHA1Digest(const SHA1Digest &)            = delete;
    SHA1Digest &operator=(const SHA1Digest &) = delete;
    SHA1Digest(SHA1Digest &&other) noexcept;
    SHA1Digest      &operator=(SHA1Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, std::size_t length) override;

private:
    std::unique_ptr<SHA1DigestImpl> _impl;
};
typedef SHA1Digest SHA1Encrypt;
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
