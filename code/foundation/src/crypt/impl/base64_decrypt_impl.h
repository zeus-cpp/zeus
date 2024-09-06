#pragma once

#include <memory>
#include <cstddef>
#include <cryptopp/base64.h>

namespace zeus
{

class Base64DecryptImpl
{
public:
    Base64DecryptImpl(bool isWebUrl);
    void             End();
    const std::byte *PlainText();
    void             Reset();
    size_t           GetSize();
    void             UpdateImpl(const void *input, size_t length);
private:
    std::unique_ptr<CryptoPP::BaseN_Decoder> _decoder;
    std::string                              _data;
    bool                                     _isWebUrl;
};
}
