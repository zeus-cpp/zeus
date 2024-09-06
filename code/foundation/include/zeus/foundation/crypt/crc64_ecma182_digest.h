#pragma once

#include <fstream>
#include <memory>
#include "zeus/foundation/crypt/base_digest.h"

namespace zeus
{
class Crc64Ecma182DigestImpl;
class Crc64Ecma182Digest : public BaseDigest
{
public:
    Crc64Ecma182Digest();
    Crc64Ecma182Digest(const void *input, size_t length);
    Crc64Ecma182Digest(const std::string &str);
    Crc64Ecma182Digest(std::ifstream &in);
    Crc64Ecma182Digest(const std::filesystem::path &path, bool binary);
    ~Crc64Ecma182Digest() override;
    Crc64Ecma182Digest(const Crc64Ecma182Digest &)            = delete;
    Crc64Ecma182Digest &operator=(const Crc64Ecma182Digest &) = delete;
    Crc64Ecma182Digest(Crc64Ecma182Digest &&other) noexcept;
    Crc64Ecma182Digest &operator=(Crc64Ecma182Digest &&other) noexcept;
    std::string         Name() override;
    const std::byte    *Digest() override;
    void                Reset() override;
    size_t              GetSize() override;
    uint64_t            DigestSum();
protected:
    void UpdateImpl(const void *input, size_t length) override;
private:
    std::unique_ptr<Crc64Ecma182DigestImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
