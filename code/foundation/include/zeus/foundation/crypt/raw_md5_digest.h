#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class RawMd5DigestImpl;
//RawMd5Digest实现使用自有代码，没有使用CryptoPP加密库
class RawMd5Digest : public BaseDigest
{
public:
    RawMd5Digest();
    RawMd5Digest(const void *input, size_t length);
    RawMd5Digest(const std::string &str);
    RawMd5Digest(std::ifstream &in);
    RawMd5Digest(const std::filesystem::path &path, bool binary);
    ~RawMd5Digest() override;
    RawMd5Digest(const RawMd5Digest &)            = delete;
    RawMd5Digest &operator=(const RawMd5Digest &) = delete;
    RawMd5Digest(RawMd5Digest &&other) noexcept;
    RawMd5Digest    &operator=(RawMd5Digest &&other) noexcept;
    std::string      Name() override;
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<RawMd5DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
