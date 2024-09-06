#include <zeus/foundation/crypt/hmac_digest.h>
#include "impl/hmac_digest_impl.h"

namespace zeus
{
HMACDigest::HMACDigest(const void* saltByte, size_t saltLength, HMACDigestType type)
    : _impl(std::make_unique<HMACDigestImpl>(type, saltByte, saltLength))
{
}
HMACDigest::HMACDigest(const void* input, size_t length, const void* saltByte, size_t saltLength, HMACDigestType type)
    : HMACDigest(saltByte, saltLength, type)
{
    Update(input, length);
}
HMACDigest::HMACDigest(const std::string& str, const void* saltByte, size_t saltLength, HMACDigestType type) : HMACDigest(saltByte, saltLength, type)
{
    Update(str);
}
HMACDigest::HMACDigest(std::ifstream& in, const void* saltByte, size_t saltLength, HMACDigestType type) : HMACDigest(saltByte, saltLength, type)
{
    Update(in);
}
HMACDigest::HMACDigest(const std::filesystem::path& path, bool binary, const void* saltByte, size_t saltLength, HMACDigestType type)
    : HMACDigest(saltByte, saltLength, type)
{
    Update(path, binary);
}
HMACDigest::~HMACDigest()
{
}
HMACDigest::HMACDigest(HMACDigest&& other) noexcept : _impl(std::move(other._impl))
{
}
HMACDigest& HMACDigest::operator=(HMACDigest&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
std::string HMACDigest::Name()
{
    return _impl->Name();
}
const std::byte* HMACDigest::Digest()
{
    return _impl->Digest();
}
void HMACDigest::Reset()
{
    _impl->Reset();
}
size_t HMACDigest::GetSize()
{
    return _impl->GetSize();
}
void HMACDigest::UpdateImpl(const void* input, size_t length)
{
    _impl->UpdateImpl(input, length);
}
} // namespace zeus
