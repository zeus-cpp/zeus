#include "zeus/foundation/crypt/md5_digest.h"
#include "impl/md5_digest_impl.h"

namespace zeus
{
Md5Digest::Md5Digest() : _impl(std::make_unique<Md5DigestImpl>())
{
}
Md5Digest::Md5Digest(const void* input, size_t length) : Md5Digest()
{
    Update(input, length);
}
Md5Digest::Md5Digest(const std::string& str) : Md5Digest()
{
    Update(str);
}
Md5Digest::Md5Digest(std::ifstream& in) : Md5Digest()
{
    Update(in);
}

Md5Digest::Md5Digest(const std::filesystem::path& path, bool binary) : Md5Digest()
{
    Update(path, binary);
}

Md5Digest::~Md5Digest()
{
}
Md5Digest::Md5Digest(Md5Digest&& other) noexcept : _impl(std::move(other._impl))
{
}
Md5Digest& Md5Digest::operator=(Md5Digest&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string Md5Digest::Name()
{
    return std::string("MD5");
}

const std::byte* Md5Digest::Digest()
{
    return _impl->Digest();
}

void Md5Digest::Reset()
{
    _impl->Reset();
}

size_t Md5Digest::GetSize()
{
    return _impl->GetSize();
}

void Md5Digest::UpdateImpl(const void* input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
