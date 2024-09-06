#include "base64_decrypt_impl.h"
#include <cassert>

namespace zeus
{
Base64DecryptImpl::Base64DecryptImpl(bool isWebUrl) : _isWebUrl(isWebUrl)
{
    Reset();
}

void Base64DecryptImpl::End()
{
    _decoder->MessageEnd();
}

const std::byte *Base64DecryptImpl::PlainText()
{
    return reinterpret_cast<const std::byte *>(_data.data());
}

void Base64DecryptImpl::Reset()
{
    if (_isWebUrl)
    {
        _decoder = std::make_unique<CryptoPP::Base64URLDecoder>(new CryptoPP::StringSink(_data));
    }
    else
    {
        _decoder = std::make_unique<CryptoPP::Base64Decoder>(new CryptoPP::StringSink(_data));
    }
    _data.clear();
}

size_t Base64DecryptImpl::GetSize()
{
    return _data.size();
}

void Base64DecryptImpl::UpdateImpl(const void *input, size_t length)
{
    _decoder->Put(reinterpret_cast<const CryptoPP::byte *>(input), length);
}
} // namespace zeus
