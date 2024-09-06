#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class Md5DigestImpl;
class Md5Digest : public BaseDigest
{
public:
    Md5Digest();
    Md5Digest(const void *input, size_t length);
    Md5Digest(const std::string &str);
    Md5Digest(std::ifstream &in);
    Md5Digest(const std::filesystem::path &path, bool binary);
    ~Md5Digest() override;
    Md5Digest(const Md5Digest &)            = delete;
    Md5Digest &operator=(const Md5Digest &) = delete;
    Md5Digest(Md5Digest &&other) noexcept;
    Md5Digest       &operator=(Md5Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<Md5DigestImpl> _impl;
};
typedef Md5Digest Md5Encrypt;
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
