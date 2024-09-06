#pragma once

#include <memory>
#include <cstddef>
#include <cryptopp/base64.h>

namespace zeus
{

class Base64EncryptImpl
{
public:
    Base64EncryptImpl(bool isWebUrl = false, size_t lineLength = 0);

    const std::byte *CipherText();
    void             Reset();
    size_t           GetSize();
    void             UpdateImpl(const void *input, size_t length);
    void             End();

private:
    std::unique_ptr<CryptoPP::SimpleProxyFilter> _encoder;
    std::string                                  _data;
    bool                                         _isWebUrl;
    size_t                                       _lineLength;
};
}
