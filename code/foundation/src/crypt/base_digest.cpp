#include "zeus/foundation/crypt/base_digest.h"
#include <array>
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/resource/file_mapping.h"

namespace zeus
{
BaseDigest::BaseDigest()
{
}

BaseDigest::~BaseDigest()
{
}

BaseDigest::BaseDigest(BaseDigest && /*other*/) noexcept
{
}

BaseDigest &BaseDigest::operator=(BaseDigest && /*other*/) noexcept
{
    return *this;
}

void BaseDigest::Update(const void *input, size_t length)
{
    UpdateImpl(input, length);
}

void BaseDigest::Update(const std::string &str)
{
    UpdateImpl(str.c_str(), str.length());
}

void BaseDigest::Update(std::ifstream &in)
{
    static constexpr size_t kBufferSize = 1024;
    if (!in)
    {
        return;
    }
    std::streamsize               length = 0;
    std::array<char, kBufferSize> buffer;
    while (!in.eof())
    {
        in.read(buffer.data(), buffer.size());
        length = in.gcount();
        if (length > 0)
        {
            UpdateImpl(buffer.data(), static_cast<size_t>(length));
        }
    }
}
void BaseDigest::Update(const std::filesystem::path &path, bool binary)
{
    static constexpr uint64_t kMappingSize = 2 * 1024 * 1024;

    if (!binary)
    {
        std::ifstream in(path, std::ios::in);
        Update(in);
        return;
    }

    do
    {
        auto mapping = FileMapping::Create(path, false);
        if (!mapping.has_value())
        {
            break;
        }

        if (mapping->FileSize() <= kMappingSize)
        {
            if (const auto ret = mapping->MapAll(); !ret.has_value())
            {
                break;
            }
            UpdateImpl(mapping->Data(), mapping->Size());
            return;
        }
        else
        {
            const uint64_t count = mapping->FileSize() / kMappingSize;
            for (uint64_t index = 0; index <= count; ++index)
            {
                if (const auto ret = mapping->Map(index * kMappingSize, count == index ? (mapping->FileSize() % kMappingSize) : kMappingSize);
                    !ret.has_value())
                {
                    return;
                }
                UpdateImpl(mapping->Data(), mapping->Size());
            }
            return;
        }
    }
    while (false);
    std::ifstream in(path, std::ios::in | std::ios::binary);
    Update(in);
}
std::string BaseDigest::ToString(bool upCase)
{
    const std::byte *pstr = Digest();
    return BytesToHexString(pstr, GetSize(), upCase);
}
} // namespace zeus
