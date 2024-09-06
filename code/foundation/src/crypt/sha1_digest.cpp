#include "zeus/foundation/crypt/sha1_digest.h"
#include "impl/sha1_digest_impl.h"

namespace zeus
{
SHA1Digest::SHA1Digest() : _impl(std::make_unique<SHA1DigestImpl>())
{
}
SHA1Digest::SHA1Digest(const void* input, size_t length) : SHA1Digest()
{
    Update(input, length);
}
SHA1Digest::SHA1Digest(const std::string& str) : SHA1Digest()
{
    Update(str);
}
SHA1Digest::SHA1Digest(std::ifstream& in) : SHA1Digest()
{
    Update(in);
}
SHA1Digest::SHA1Digest(const std::filesystem::path& path, bool binary) : SHA1Digest()
{
    Update(path, binary);
}
SHA1Digest::~SHA1Digest()
{
}

SHA1Digest::SHA1Digest(SHA1Digest&& other) noexcept : _impl(std::move(other._impl))
{
}
SHA1Digest& SHA1Digest::operator=(SHA1Digest&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string SHA1Digest::Name()
{
    return std::string("SHA1");
}
const std::byte* SHA1Digest::Digest()
{
    return _impl->Digest();
}
void SHA1Digest::Reset()
{
    _impl->Reset();
}

size_t SHA1Digest::GetSize()
{
    return _impl->GetSize();
}
void SHA1Digest::UpdateImpl(const void* input, std::size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
