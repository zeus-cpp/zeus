#include "zeus/foundation/crypt/raw_md5_digest.h"
#include "impl/raw_md5_digest_impl.h"

namespace zeus
{
RawMd5Digest::RawMd5Digest() : _impl(std::make_unique<RawMd5DigestImpl>())
{
}
RawMd5Digest::RawMd5Digest(const void* input, size_t length) : RawMd5Digest()
{
    Update(input, length);
}
RawMd5Digest::RawMd5Digest(const std::string& str) : RawMd5Digest()
{
    Update(str);
}
RawMd5Digest::RawMd5Digest(std::ifstream& in) : RawMd5Digest()
{
    Update(in);
}

RawMd5Digest::RawMd5Digest(const std::filesystem::path& path, bool binary) : RawMd5Digest()
{
    Update(path, binary);
}

RawMd5Digest::~RawMd5Digest()
{
}
RawMd5Digest::RawMd5Digest(RawMd5Digest&& other) noexcept : _impl(std::move(other._impl))
{
}
RawMd5Digest& RawMd5Digest::operator=(RawMd5Digest&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string RawMd5Digest::Name()
{
    return std::string("MD5");
}

const std::byte* RawMd5Digest::Digest()
{
    return _impl->Digest();
}

void RawMd5Digest::Reset()
{
    _impl->Reset();
}

size_t RawMd5Digest::GetSize()
{
    return _impl->GetSize();
}

void RawMd5Digest::UpdateImpl(const void* input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
