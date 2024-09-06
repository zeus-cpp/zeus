#include "aes_decrypt_impl.h"
#include <stdexcept>
#include <cassert>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include "zeus/foundation/crypt/padding.h"
#include "crypt_name.hpp"

using namespace CryptoPP;

namespace zeus
{

AesDecryptImpl::AesDecryptImpl(AESMode mode, AESPadding padding, const std::string& key, const std::string& iv)
    : _mode(mode), _padding(padding), _key(key), _iv(iv)
{
    Reset();
}

const std::byte* AesDecryptImpl::PlainText()
{
    return reinterpret_cast<const std::byte*>(_data.data());
}

void AesDecryptImpl::Reset()
{
    switch (_mode)
    {
    case AESMode::CBC:
        _decrytption = std::make_unique<CBC_Mode<CryptoPP::AES>::Decryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    case AESMode::CFB:
        _decrytption = std::make_unique<CFB_Mode<CryptoPP::AES>::Decryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    case AESMode::CTR:
        _decrytption = std::make_unique<CTR_Mode<CryptoPP::AES>::Decryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    case AESMode::ECB:
        _decrytption = std::make_unique<ECB_Mode<CryptoPP::AES>::Decryption>(reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size());
        break;
    case AESMode::OFB:
        _decrytption = std::make_unique<OFB_Mode<CryptoPP::AES>::Decryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    default:
        throw std::invalid_argument("Illegal AES Mode");
    }
    _filter =
        std::make_unique<StreamTransformationFilter>(*_decrytption, new CryptoPP::StringSink(_data), CryptoPP::BlockPaddingSchemeDef::NO_PADDING);
    _data.clear();
}

size_t AesDecryptImpl::GetSize()
{
    return _data.size();
}

void AesDecryptImpl::UpdateImpl(const void* input, size_t length)
{
    _filter->Put(static_cast<const CryptoPP::byte*>(input), length);
}

void AesDecryptImpl::End()
{
    _filter->MessageEnd();
    size_t paddingLength = 0;
    switch (_padding)
    {
    case AESPadding::NONE:
        break;
    case AESPadding::PKCS7:
        paddingLength = FindPKCS7Padding(reinterpret_cast<const uint8_t*>(_data.data()), _data.size());
        break;
    case AESPadding::ANSI_X_923:
        paddingLength = FindANSI_X_923Padding(reinterpret_cast<const uint8_t*>(_data.data()), _data.size());
        break;
    case AESPadding::ISO_10126:
        paddingLength = FindISO_10126Padding(reinterpret_cast<const uint8_t*>(_data.data()), _data.size());
        break;
    default:
        break;
    }
    if (paddingLength && _data.size() >= paddingLength)
    {
        _data.erase(_data.size() - paddingLength);
    }
}

std::string AesDecryptImpl::Name()
{
    return "AES(" + AESModeName(_mode) + "|" + AESPaddingName(_padding) + ")";
}

} // namespace zeus