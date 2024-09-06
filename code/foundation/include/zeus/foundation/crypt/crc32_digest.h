#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class Crc32DigestImpl;
class Crc32Digest : public BaseDigest
{
public:
    Crc32Digest();
    Crc32Digest(const void *input, size_t length);
    Crc32Digest(const std::string &str);
    Crc32Digest(std::ifstream &in);
    Crc32Digest(const std::filesystem::path &path, bool binary);
    ~Crc32Digest() override;
    Crc32Digest(const Crc32Digest &)            = delete;
    Crc32Digest &operator=(const Crc32Digest &) = delete;
    Crc32Digest(Crc32Digest &&other) noexcept;
    Crc32Digest     &operator=(Crc32Digest &&other) noexcept;
    std::string      Name() override;
    //注意，因为crc32的计算结果是整数，所以其二进制结果会受到大小端的影响
    const std::byte *Digest() override;
    void             Reset() override;
    size_t           GetSize() override;
    uint32_t         DigestSum();
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<Crc32DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
