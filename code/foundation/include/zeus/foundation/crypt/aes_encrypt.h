#pragma once

#include <memory>
#include "zeus/foundation/crypt/base_encrypt.h"
#include "zeus/foundation/crypt/crypt_common.h"

namespace zeus
{
class AesEncryptImpl;
//使用前请阅读BaseEncrypt中的使用方法说明
class AesEncrypt : public BaseEncrypt
{
public:
    AesEncrypt(AESMode mode, AESPadding padding, const std::string &key, const std::string &iv = "");
    AesEncrypt(const void *input, size_t length, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    AesEncrypt(const std::string &str, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    AesEncrypt(std::ifstream &in, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    AesEncrypt(const std::filesystem::path &path, bool binary, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    ~AesEncrypt() override;
    AesEncrypt(const AesEncrypt &)            = delete;
    AesEncrypt &operator=(const AesEncrypt &) = delete;
    AesEncrypt(AesEncrypt &&other) noexcept;
    AesEncrypt      &operator=(AesEncrypt &&other) noexcept;
    std::string      Name() override;
    const std::byte *CipherText() override;
    void             Reset() override;
    size_t           GetSize() override;
    void             End() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;

private:

private:
    std::unique_ptr<AesEncryptImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
