#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class SHA512DigestImpl;
class SHA512Digest : public BaseDigest
{
public:
    SHA512Digest();
    SHA512Digest(const void *input, size_t length);
    SHA512Digest(const std::string &str);
    SHA512Digest(std::ifstream &in);
    SHA512Digest(const std::filesystem::path &path, bool binary);
    ~SHA512Digest() override;
    SHA512Digest(const SHA512Digest &)            = delete;
    SHA512Digest &operator=(const SHA512Digest &) = delete;
    SHA512Digest(SHA512Digest &&other) noexcept;
    SHA512Digest    &operator=(SHA512Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<SHA512DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
