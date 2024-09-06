#include "zeus/foundation/crypt/crc64_ecma182_digest.h"
#include "impl/crc64_ecma182_digest_impl.h"

namespace zeus
{
Crc64Ecma182Digest::Crc64Ecma182Digest() : _impl(std::make_unique<Crc64Ecma182DigestImpl>())
{
    Reset();
}

Crc64Ecma182Digest::Crc64Ecma182Digest(const void *input, size_t length) : Crc64Ecma182Digest()
{
    Update(input, length);
}

Crc64Ecma182Digest::Crc64Ecma182Digest(const std::string &str) : Crc64Ecma182Digest()
{
    Update(str);
}

Crc64Ecma182Digest::Crc64Ecma182Digest(std::ifstream &in) : Crc64Ecma182Digest()
{
    Update(in);
}

Crc64Ecma182Digest::Crc64Ecma182Digest(const std::filesystem::path &path, bool binary) : Crc64Ecma182Digest()
{
    Update(path, binary);
}

Crc64Ecma182Digest::~Crc64Ecma182Digest()
{
}

Crc64Ecma182Digest::Crc64Ecma182Digest(Crc64Ecma182Digest &&other) noexcept : _impl(std::move(other._impl))
{
}
Crc64Ecma182Digest &Crc64Ecma182Digest::operator=(Crc64Ecma182Digest &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string Crc64Ecma182Digest::Name()
{
    return std::string("CRC64ECMA182");
}

const std::byte *Crc64Ecma182Digest::Digest()
{
    return _impl->Digest();
}

size_t Crc64Ecma182Digest::GetSize()
{
    return _impl->GetSize();
}

uint64_t Crc64Ecma182Digest::DigestSum()
{
    return *reinterpret_cast<const uint64_t *>(_impl->Digest());
}

void Crc64Ecma182Digest::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}

void Crc64Ecma182Digest::Reset()
{
    _impl->Reset();
}
} // namespace zeus
