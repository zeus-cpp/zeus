#include "zeus/foundation/crypt/sha224_digest.h"
#include "impl/sha224_digest_impl.h"

namespace zeus
{
SHA224Digest::SHA224Digest() : _impl(std::make_unique<SHA224DigestImpl>())
{
    Reset();
}

SHA224Digest::SHA224Digest(const void *input, size_t length) : SHA224Digest()
{
    Update(input, length);
}

SHA224Digest::SHA224Digest(const std::string &str) : SHA224Digest()
{
    Update(str);
}

SHA224Digest::SHA224Digest(std::ifstream &in) : SHA224Digest()
{
    Update(in);
}

SHA224Digest::SHA224Digest(const std::filesystem::path &path, bool binary) : SHA224Digest()
{
    Update(path, binary);
}

SHA224Digest::~SHA224Digest()
{
}

SHA224Digest::SHA224Digest(SHA224Digest &&other) noexcept : _impl(std::move(other._impl))
{
}
SHA224Digest &SHA224Digest::operator=(SHA224Digest &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string SHA224Digest::Name()
{
    return std::string("SHA224");
}

const std::byte *SHA224Digest::Digest()
{
    return _impl->Digest();
}

size_t SHA224Digest::GetSize()
{
    return _impl->GetSize();
}

void SHA224Digest::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}

void SHA224Digest::Reset()
{
    _impl->Reset();
}
} // namespace zeus
