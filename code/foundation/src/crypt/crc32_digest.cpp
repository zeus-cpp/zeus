#include "zeus/foundation/crypt/crc32_digest.h"
#include "impl/crc32_digest_impl.h"

namespace zeus
{
Crc32Digest::Crc32Digest() : _impl(std::make_unique<Crc32DigestImpl>())
{
    Reset();
}

Crc32Digest::Crc32Digest(const void *input, size_t length) : Crc32Digest()
{
    Update(input, length);
}

Crc32Digest::Crc32Digest(const std::string &str) : Crc32Digest()
{
    Update(str);
}

Crc32Digest::Crc32Digest(std::ifstream &in) : Crc32Digest()
{
    Update(in);
}

Crc32Digest::Crc32Digest(const std::filesystem::path &path, bool binary) : Crc32Digest()
{
    Update(path, binary);
}

Crc32Digest::~Crc32Digest()
{
}

Crc32Digest::Crc32Digest(Crc32Digest &&other) noexcept : _impl(std::move(other._impl))
{
}
Crc32Digest &Crc32Digest::operator=(Crc32Digest &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string Crc32Digest::Name()
{
    return std::string("CRC32");
}

const std::byte *Crc32Digest::Digest()
{
    return _impl->Digest();
}

size_t Crc32Digest::GetSize()
{
    return _impl->GetSize();
}

uint32_t Crc32Digest::DigestSum()
{
    return *reinterpret_cast<const uint32_t *>(_impl->Digest());
}

void Crc32Digest::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}

void Crc32Digest::Reset()
{
    _impl->Reset();
}
} // namespace zeus
