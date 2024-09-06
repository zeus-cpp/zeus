#include "zeus/foundation/crypt/base64_encrypt.h"
#include "impl/base64_encrypt_impl.h"

using namespace std;

namespace zeus
{
Base64Encrypt::Base64Encrypt(bool isWebUrl, size_t lineLength)
{
    _impl = std::make_unique<Base64EncryptImpl>(isWebUrl, lineLength);
}

Base64Encrypt::Base64Encrypt(const void *input, size_t length, bool isWebUrl, size_t lineLength) : Base64Encrypt(isWebUrl, lineLength)
{
    Update(input, length);
}

Base64Encrypt::Base64Encrypt(const string &str, bool isWebUrl, size_t lineLength) : Base64Encrypt(isWebUrl, lineLength)
{
    Update(str);
}

Base64Encrypt::Base64Encrypt(ifstream &in, bool isWebUrl, size_t lineLength) : Base64Encrypt(isWebUrl, lineLength)
{
    Update(in);
}

Base64Encrypt::Base64Encrypt(const std::filesystem::path &path, bool binary, bool isWebUrl, size_t lineLength) : Base64Encrypt(isWebUrl, lineLength)
{
    Update(path, binary);
}

Base64Encrypt::~Base64Encrypt()
{
}

Base64Encrypt::Base64Encrypt(Base64Encrypt &&other) noexcept : _impl(std::move(other._impl))
{
}
Base64Encrypt &Base64Encrypt::operator=(Base64Encrypt &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string Base64Encrypt::Name()
{
    return std::string("BASE64");
}

const std::byte *Base64Encrypt::CipherText()
{
    return _impl->CipherText();
}

void Base64Encrypt::Reset()
{
    _impl->Reset();
}

size_t Base64Encrypt::GetSize()
{
    return _impl->GetSize();
}

void Base64Encrypt::End()
{
    _impl->End();
}

void Base64Encrypt::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
