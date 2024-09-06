#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class SHA224DigestImpl;
class SHA224Digest : public BaseDigest
{
public:
    SHA224Digest();
    SHA224Digest(const void *input, size_t length);
    SHA224Digest(const std::string &str);
    SHA224Digest(std::ifstream &in);
    SHA224Digest(const std::filesystem::path &path, bool binary);
    ~SHA224Digest() override;
    SHA224Digest(const SHA224Digest &)            = delete;
    SHA224Digest &operator=(const SHA224Digest &) = delete;
    SHA224Digest(SHA224Digest &&other) noexcept;
    SHA224Digest    &operator=(SHA224Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<SHA224DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
