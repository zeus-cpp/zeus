#pragma once

#include <memory>
#include <cstddef>
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include "zeus/foundation/crypt/crypt_common.h"

namespace zeus
{

class AesEncryptImpl
{
public:
    AesEncryptImpl(AESMode mode, AESPadding padding, const std::string &key, const std::string &iv);

    const std::byte *CipherText();
    void             Reset();
    size_t           GetSize();
    void             UpdateImpl(const void *input, size_t length);
    void             End();
    std::string      Name();
private:
    std::unique_ptr<CryptoPP::StreamTransformation>       _encrytption;
    std::unique_ptr<CryptoPP::StreamTransformationFilter> _filter;
    size_t                                                _processedSize = 0;
    std::string                                           _data;
    AESMode                                               _mode;
    AESPadding                                            _padding;
    std::string                                           _key;
    std::string                                           _iv;
};
}
