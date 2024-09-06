#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class SHA384DigestImpl;
class SHA384Digest : public BaseDigest
{
public:
    SHA384Digest();
    SHA384Digest(const void *input, size_t length);
    SHA384Digest(const std::string &str);
    SHA384Digest(std::ifstream &in);
    SHA384Digest(const std::filesystem::path &path, bool binary);
    ~SHA384Digest() override;
    SHA384Digest(const SHA384Digest &)            = delete;
    SHA384Digest &operator=(const SHA384Digest &) = delete;
    SHA384Digest(SHA384Digest &&other) noexcept;
    SHA384Digest    &operator=(SHA384Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<SHA384DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
