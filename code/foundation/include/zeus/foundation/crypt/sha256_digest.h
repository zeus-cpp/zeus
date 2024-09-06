#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class SHA256DigestImpl;
class SHA256Digest : public BaseDigest
{
public:
    SHA256Digest();
    SHA256Digest(const void *input, size_t length);
    SHA256Digest(const std::string &str);
    SHA256Digest(std::ifstream &in);
    SHA256Digest(const std::filesystem::path &path, bool binary);
    ~SHA256Digest() override;
    SHA256Digest(const SHA256Digest &)            = delete;
    SHA256Digest &operator=(const SHA256Digest &) = delete;
    SHA256Digest(SHA256Digest &&other) noexcept;
    SHA256Digest    &operator=(SHA256Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<SHA256DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
