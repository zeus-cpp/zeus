#include "zeus/foundation/crypt/sha512_digest.h"
#include "impl/sha512_digest_impl.h"

namespace zeus
{
SHA512Digest::SHA512Digest() : _impl(std::make_unique<SHA512DigestImpl>())
{
    Reset();
}

SHA512Digest::SHA512Digest(const void *input, size_t length) : SHA512Digest()
{
    Update(input, length);
}

SHA512Digest::SHA512Digest(const std::string &str) : SHA512Digest()
{
    Update(str);
}

SHA512Digest::SHA512Digest(std::ifstream &in) : SHA512Digest()
{
    Update(in);
}

SHA512Digest::SHA512Digest(const std::filesystem::path &path, bool binary) : SHA512Digest()
{
    Update(path, binary);
}

SHA512Digest::~SHA512Digest()
{
}

SHA512Digest::SHA512Digest(SHA512Digest &&other) noexcept : _impl(std::move(other._impl))
{
}
SHA512Digest &SHA512Digest::operator=(SHA512Digest &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string SHA512Digest::Name()
{
    return std::string("SHA512");
}

const std::byte *SHA512Digest::Digest()
{
    return _impl->Digest();
}

size_t SHA512Digest::GetSize()
{
    return _impl->GetSize();
}

void SHA512Digest::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}

void SHA512Digest::Reset()
{
    _impl->Reset();
}
} // namespace zeus
