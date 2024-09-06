#include "aes_encrypt_impl.h"
#include <stdexcept>
#include <cassert>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include "zeus/foundation/crypt/padding.h"
#include "crypt_name.hpp"

using namespace CryptoPP;

namespace zeus
{

namespace
{
const uint8_t kAesBlockSize = 16;
}

AesEncryptImpl::AesEncryptImpl(AESMode mode, AESPadding padding, const std::string& key, const std::string& iv)
    : _mode(mode), _padding(padding), _key(key), _iv(iv)
{
    Reset();
}

const std::byte* AesEncryptImpl::CipherText()
{
    return reinterpret_cast<const std::byte*>(_data.data());
}

void AesEncryptImpl::Reset()
{
    switch (_mode)
    {
    case AESMode::CBC:
        _encrytption = std::make_unique<CBC_Mode<CryptoPP::AES>::Encryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    case AESMode::CFB:
        _encrytption = std::make_unique<CFB_Mode<CryptoPP::AES>::Encryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    case AESMode::CTR:
        _encrytption = std::make_unique<CTR_Mode<CryptoPP::AES>::Encryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    case AESMode::ECB:
        _encrytption = std::make_unique<ECB_Mode<CryptoPP::AES>::Encryption>(reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size());
        break;
    case AESMode::OFB:
        _encrytption = std::make_unique<OFB_Mode<CryptoPP::AES>::Encryption>(
            reinterpret_cast<const CryptoPP::byte*>(_key.data()), _key.size(), reinterpret_cast<const CryptoPP::byte*>(_iv.data())
        );
        break;
    default:
        throw std::invalid_argument("Illegal AES Mode");
    }
    _filter =
        std::make_unique<StreamTransformationFilter>(*_encrytption, new CryptoPP::StringSink(_data), CryptoPP::BlockPaddingSchemeDef::NO_PADDING);
    _data.clear();
    _processedSize = 0;
}

size_t AesEncryptImpl::GetSize()
{
    return _data.size();
}

void AesEncryptImpl::UpdateImpl(const void* input, size_t length)
{
    _filter->Put(reinterpret_cast<const CryptoPP::byte*>(input), length);
    _processedSize += length;
}

void AesEncryptImpl::End()
{
    uint8_t paddingBuffer[kAesBlockSize] = {};
    size_t  paddingLength                = 0;
    switch (_padding)
    {
    case AESPadding::NONE:
        break;
    case AESPadding::PKCS7:
        paddingLength = FillPKCS7Padding(paddingBuffer, kAesBlockSize, _processedSize, kAesBlockSize);
        break;
    case AESPadding::ANSI_X_923:
        paddingLength = FillANSI_X_923Padding(paddingBuffer, kAesBlockSize, _processedSize, kAesBlockSize);
        break;
    case AESPadding::ISO_10126:
        paddingLength = FillISO_10126Padding(paddingBuffer, kAesBlockSize, _processedSize, kAesBlockSize);
        break;
    default:
        break;
    }
    _filter->Put(paddingBuffer, paddingLength);
    _filter->MessageEnd();
}

std::string AesEncryptImpl::Name()
{
    return "AES(" + AESModeName(_mode) + "|" + AESPaddingName(_padding) + ")";
}

} // namespace zeus