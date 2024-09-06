#include "zeus/foundation/crypt/sha384_digest.h"
#include "impl/sha384_digest_impl.h"

namespace zeus
{
SHA384Digest::SHA384Digest() : _impl(std::make_unique<SHA384DigestImpl>())
{
    Reset();
}

SHA384Digest::SHA384Digest(const void *input, size_t length) : SHA384Digest()
{
    Update(input, length);
}

SHA384Digest::SHA384Digest(const std::string &str) : SHA384Digest()
{
    Update(str);
}

SHA384Digest::SHA384Digest(std::ifstream &in) : SHA384Digest()
{
    Update(in);
}

SHA384Digest::SHA384Digest(const std::filesystem::path &path, bool binary) : SHA384Digest()
{
    Update(path, binary);
}

SHA384Digest::~SHA384Digest()
{
}

SHA384Digest::SHA384Digest(SHA384Digest &&other) noexcept : _impl(std::move(other._impl))
{
}
SHA384Digest &SHA384Digest::operator=(SHA384Digest &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string SHA384Digest::Name()
{
    return std::string("SHA384");
}

const std::byte *SHA384Digest::Digest()
{
    return _impl->Digest();
}

size_t SHA384Digest::GetSize()
{
    return _impl->GetSize();
}

void SHA384Digest::UpdateImpl(const void *input, size_t length)
{
    _impl->UpdateImpl(input, length);
}

void SHA384Digest::Reset()
{
    _impl->Reset();
}
} // namespace zeus
