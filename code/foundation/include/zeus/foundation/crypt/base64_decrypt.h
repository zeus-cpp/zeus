#pragma once

#include <memory>

#include "zeus/foundation/crypt/base_decrypt.h"

namespace zeus
{
class Base64DecryptImpl;
class Base64Decrypt : public BaseDecrypt
{
public:
    Base64Decrypt(bool isWebUrl = false);
    Base64Decrypt(const void *input, size_t length, bool isWebUrl = false);
    Base64Decrypt(const std::string &str, bool isWebUrl = false);
    Base64Decrypt(std::ifstream &in, bool isWebUrl = false);
    Base64Decrypt(const std::filesystem::path &path, bool binary, bool isWebUrl = false);
    ~Base64Decrypt() override;
    Base64Decrypt(const Base64Decrypt &)            = delete;
    Base64Decrypt &operator=(const Base64Decrypt &) = delete;
    Base64Decrypt(Base64Decrypt &&other) noexcept;
    Base64Decrypt   &operator=(Base64Decrypt &&other) noexcept;
    std::string      Name() override;
    const std::byte *PlainText() override;
    void             Reset() override;
    size_t           GetSize() override;
    void             End() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;

private:
    std::unique_ptr<Base64DecryptImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
