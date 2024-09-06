#include "base64_encrypt_impl.h"
#include <cassert>

namespace zeus
{
Base64EncryptImpl::Base64EncryptImpl(bool isWebUrl, size_t lineLength) : _isWebUrl(isWebUrl), _lineLength(lineLength)
{
    Reset();
}
const std::byte *Base64EncryptImpl::CipherText()
{
    return reinterpret_cast<const std::byte *>(_data.data());
}
void Base64EncryptImpl::Reset()
{
    if (_isWebUrl)
    {
        _encoder = std::make_unique<CryptoPP::Base64URLEncoder>(new CryptoPP::StringSink(_data));
    }
    else
    {
        _encoder = std::make_unique<CryptoPP::Base64Encoder>(new CryptoPP::StringSink(_data), _lineLength > 0, _lineLength > 0 ? _lineLength : 0);
    }
    _data.clear();
}
size_t Base64EncryptImpl::GetSize()
{
    return _data.size();
}
void Base64EncryptImpl::UpdateImpl(const void *input, size_t length)
{
    _encoder->Put(reinterpret_cast<const CryptoPP::byte *>(input), length);
}
void Base64EncryptImpl::End()
{
    _encoder->MessageEnd();
}
} // namespace zeus
