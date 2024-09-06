#pragma once

#include <memory>
#include "zeus/foundation/crypt/base_decrypt.h"
#include "zeus/foundation/crypt/crypt_common.h"

namespace zeus
{
class AesDecryptImpl;
//使用前请阅读BaseDecrypt中的使用方法说明
class AesDecrypt : public BaseDecrypt
{
public:
    AesDecrypt(AESMode mode, AESPadding padding, const std::string &key, const std::string &iv = "");
    AesDecrypt(const void *input, size_t length, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    AesDecrypt(const std::string &str, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    AesDecrypt(std::ifstream &in, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    AesDecrypt(const std::filesystem::path &path, bool binary, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);
    ~AesDecrypt() override;
    AesDecrypt(const AesDecrypt &)            = delete;
    AesDecrypt &operator=(const AesDecrypt &) = delete;
    AesDecrypt(AesDecrypt &&other) noexcept;
    AesDecrypt      &operator=(AesDecrypt &&other) noexcept;
    std::string      Name() override;
    const std::byte *PlainText() override;
    void             Reset() override;
    size_t           GetSize() override;
    void             End() override;
protected:
    void UpdateImpl(const void *input, size_t length) override;

private:

private:
    std::unique_ptr<AesDecryptImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
