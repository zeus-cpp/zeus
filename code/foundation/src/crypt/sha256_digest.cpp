#include "zeus/foundation/crypt/sha256_digest.h"
#include "impl/sha256_digest_impl.h"

namespace zeus
{
SHA256Digest::SHA256Digest() : _impl(std::make_unique<SHA256DigestImpl>())
{
    Reset();
}

SHA256Digest::SHA256Digest(const void *input, size_t length) : SHA256Digest()
{
    Update(input, length);
}

SHA256Digest::SHA256Digest(const std::string &str) : SHA256Digest()
{
    Update(str);
}

SHA256Digest::SHA256Digest(std::ifstream &in) : SHA256Digest()
{
    Update(in);
}

SHA256Digest::SHA256Digest(const std::filesystem::path &path, bool binary) : SHA256Digest()
{
    Update(path, binary);
}

SHA256Digest::~SHA256Digest()
{
}

SHA256Digest::SHA256Digest(SHA256Digest &&other) noexcept : _impl(std::move(other._impl))
{
}
SHA256Digest &SHA256Digest::operator=(SHA256Digest &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string SHA256Digest::Name()
{
    return std::string("SHA256");
}

const std::byte *SHA256Digest::Digest()
{
    return _impl->Digest();
}

size_t SHA256Digest::GetSize()
{
    return _impl->GetSize();
}

void SHA256Digest::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}

void SHA256Digest::Reset()
{
    _impl->Reset();
}
} // namespace zeus
