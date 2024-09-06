#pragma once

#include <memory>

#include "zeus/foundation/crypt/base_encrypt.h"

namespace zeus
{
class Base64EncryptImpl;
//使用前请阅读BaseEncrypt中的使用方法说明
class Base64Encrypt : public BaseEncrypt
{
public:
    Base64Encrypt(bool isWebUrl = false, size_t lineLength = 0);
    Base64Encrypt(const void *input, size_t length, bool isWebUrl = false, size_t lineLength = 0);
    Base64Encrypt(const std::string &str, bool isWebUrl = false, size_t lineLength = 0);
    Base64Encrypt(std::ifstream &in, bool isWebUrl = false, size_t lineLength = 0);
    Base64Encrypt(const std::filesystem::path &path, bool binary, bool isWebUrl = false, size_t lineLength = 0);
    ~Base64Encrypt() override;
    Base64Encrypt(const Base64Encrypt &)            = delete;
    Base64Encrypt &operator=(const Base64Encrypt &) = delete;
    Base64Encrypt(Base64Encrypt &&other) noexcept;
    Base64Encrypt   &operator=(Base64Encrypt &&other) noexcept;
    std::string      Name() override;
    const std::byte *CipherText() override;
    void             Reset() override;
    size_t           GetSize() override;
    void             End() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;

private:

private:
    std::unique_ptr<Base64EncryptImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
