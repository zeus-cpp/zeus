#include "zeus/foundation/crypt/base64_decrypt.h"
#include "impl/base64_decrypt_impl.h"

using namespace std;

namespace zeus
{
Base64Decrypt::Base64Decrypt(bool isWebUrl)
{
    _impl = std::make_unique<Base64DecryptImpl>(isWebUrl);
}

Base64Decrypt::Base64Decrypt(const void *input, size_t length, bool isWebUrl) : Base64Decrypt(isWebUrl)
{
    Update(input, length);
}

Base64Decrypt::Base64Decrypt(const string &str, bool isWebUrl) : Base64Decrypt(isWebUrl)
{
    Update(str);
}

Base64Decrypt::Base64Decrypt(ifstream &in, bool isWebUrl) : Base64Decrypt(isWebUrl)
{
    Update(in);
}

Base64Decrypt::Base64Decrypt(const std::filesystem::path &path, bool binary, bool isWebUrl) : Base64Decrypt(isWebUrl)
{
    Update(path, binary);
}

Base64Decrypt::~Base64Decrypt()
{
}

Base64Decrypt::Base64Decrypt(Base64Decrypt &&other) noexcept : _impl(std::move(other._impl))
{
}
Base64Decrypt &Base64Decrypt::operator=(Base64Decrypt &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string Base64Decrypt::Name()
{
    return std::string("BASE64");
}

const std::byte *Base64Decrypt::PlainText()
{
    return _impl->PlainText();
}

void Base64Decrypt::Reset()
{
    _impl->Reset();
}

size_t Base64Decrypt::GetSize()
{
    return _impl->GetSize();
}

void Base64Decrypt::End()
{
    _impl->End();
}

void Base64Decrypt::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
