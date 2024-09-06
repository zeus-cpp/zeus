#include "zeus/foundation/crypt/aes_encrypt.h"
#include "impl/aes_encrypt_impl.h"

using namespace std;

namespace zeus
{
AesEncrypt::AesEncrypt(AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
{
    _impl = std::make_unique<AesEncryptImpl>(mode, padding, key, iv);
}

AesEncrypt::AesEncrypt(const void *input, size_t length, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
    : AesEncrypt(mode, padding, key, iv)
{
    Update(input, length);
}

AesEncrypt::AesEncrypt(const string &str, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
    : AesEncrypt(mode, padding, key, iv)
{
    Update(str);
}

AesEncrypt::AesEncrypt(ifstream &in, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
    : AesEncrypt(mode, padding, key, iv)
{
    Update(in);
}

AesEncrypt::AesEncrypt(
    const std::filesystem::path &path, bool binary, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv
)
    : AesEncrypt(mode, padding, key, iv)
{
    Update(path, binary);
}

AesEncrypt::~AesEncrypt()
{
}

AesEncrypt::AesEncrypt(AesEncrypt &&other) noexcept : _impl(std::move(other._impl))
{
}
AesEncrypt &AesEncrypt::operator=(AesEncrypt &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string AesEncrypt::Name()
{
    return _impl->Name();
}

const std::byte *AesEncrypt::CipherText()
{
    return _impl->CipherText();
}

void AesEncrypt::Reset()
{
    _impl->Reset();
}

size_t AesEncrypt::GetSize()
{
    return _impl->GetSize();
}

void AesEncrypt::End()
{
    _impl->End();
}

void AesEncrypt::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
