#include "base_digest_impl.h"
#include <cstring>
namespace zeus
{
BaseDigestImpl::BaseDigestImpl(std::shared_ptr<CryptoPP::HashTransformation> algorithm) : _algorithm(algorithm)
{
    _digest = std::make_unique<CryptoPP::byte[]>(GetSize());
    Reset();
}
BaseDigestImpl::~BaseDigestImpl()
{
}
const std::byte* BaseDigestImpl::Digest()
{
    if (!_finished)
    {
        _finished = true;
        _algorithm->Final(_digest.get());
    }
    return reinterpret_cast<std::byte*>(_digest.get());
}
void BaseDigestImpl::Reset()
{
    _finished = false;
    std::memset(_digest.get(), 0, GetSize());
    _algorithm->Restart();
}
size_t BaseDigestImpl::GetSize()
{
    return _algorithm->DigestSize();
}
void BaseDigestImpl::UpdateImpl(const void* input, size_t length)
{
    _algorithm->Update(reinterpret_cast<const CryptoPP::byte*>(input), length);
}
} // namespace zeus
