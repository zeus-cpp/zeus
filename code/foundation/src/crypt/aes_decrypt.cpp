#include "zeus/foundation/crypt/aes_decrypt.h"
#include "impl/aes_decrypt_impl.h"

using namespace std;

namespace zeus
{
AesDecrypt::AesDecrypt(AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
{
    _impl = std::make_unique<AesDecryptImpl>(mode, padding, key, iv);
}

AesDecrypt::AesDecrypt(const void *input, size_t length, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
    : AesDecrypt(mode, padding, key, iv)
{
    Update(input, length);
}

AesDecrypt::AesDecrypt(const string &str, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
    : AesDecrypt(mode, padding, key, iv)
{
    Update(str);
}

AesDecrypt::AesDecrypt(ifstream &in, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv)
    : AesDecrypt(mode, padding, key, iv)
{
    Update(in);
}

AesDecrypt::AesDecrypt(
    const std::filesystem::path &path, bool binary, AESMode mode, AESPadding padding, const std::string &key, const std::string &iv
)
    : AesDecrypt(mode, padding, key, iv)
{
    Update(path, binary);
}

AesDecrypt::~AesDecrypt()
{
}

AesDecrypt::AesDecrypt(AesDecrypt &&other) noexcept : _impl(std::move(other._impl))
{
}
AesDecrypt &AesDecrypt::operator=(AesDecrypt &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string AesDecrypt::Name()
{
    return _impl->Name();
}

const std::byte *AesDecrypt::PlainText()
{
    return _impl->PlainText();
}

void AesDecrypt::Reset()
{
    _impl->Reset();
}

size_t AesDecrypt::GetSize()
{
    return _impl->GetSize();
}

void AesDecrypt::End()
{
    _impl->End();
}

void AesDecrypt::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
