#include "zeus/foundation/byte/byte_utils.h"
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic) 没有c++20，没有std::span可用
#include <string>
#include <cctype>
#include <vector>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>

namespace zeus
{
std::vector<ByteBufferView> ByteSplit(const ByteBufferView& src, const ByteBufferView& delim, size_t stride)
{
    std::vector<ByteBufferView> result;
    size_t                      offset     = 0;
    size_t                      findOffset = 0;
    while ((findOffset = ByteFind(src, delim, offset, stride)) != std::string::npos)
    {
        if (findOffset != offset)
        {
            result.emplace_back(src.SubView(offset, findOffset - offset));
        }
        offset = findOffset + delim.Size();
    }
    if (offset < src.Size())
    {
        result.emplace_back(src.SubView(offset, src.Size() - offset));
    }
    return result;
}
std::vector<uint8_t> ByteJoin(std::vector<ByteBufferView>& src, const ByteBufferView& delim)
{
    std::vector<uint8_t> result;
    if (src.empty())
    {
        return result;
    }

    size_t size = 0;

    if (!src.empty())
    {
        size = (src.size() - 1) * delim.Size();
    }

    for (auto& item : src)
    {
        size += item.Size();
    }
    result.resize(size);
    auto* data = result.data();
    for (size_t i = 0; i < src.size(); i++)
    {
        if (i)
        {
            std::memcpy(data, delim.Data(), delim.Size());
            data += delim.Size();
        }
        std::memcpy(data, src[i].Data(), src[i].Size());
        data += src[i].Size();
    }
    return result;
}

std::string::size_type ByteFind(const ByteBufferView& src, const ByteBufferView& sub, size_t offset, size_t stride)
{
    if (offset >= src.Size())
    {
        return std::string::npos;
    }
    auto element = src.SubView(offset, src.Size() - offset);
    if (!element || !sub || element.Size() < sub.Size())
    {
        return std::string::npos;
    }
    const uint8_t* start = element.Data();
    const uint8_t* end   = start + element.Size() - sub.Size();
    while (start <= end)
    {
        if (0 == std::memcmp(start, sub.Data(), sub.Size()))
        {
            return start - src.Data();
        }
        else
        {
            start += stride;
        }
    }
    return std::string::npos;
}
std::string::size_type ByteReverseFind(const ByteBufferView& src, const ByteBufferView& sub, size_t offset, size_t stride)
{
    if (offset >= src.Size())
    {
        return std::string::npos;
    }
    auto element = src.SubView(offset, src.Size() - offset);
    if (!element || !sub || element.Size() < sub.Size())
    {
        return std::string::npos;
    }
    const uint8_t* start = element.Data();
    const uint8_t* end   = start + element.Size() - sub.Size();
    while (end >= start)
    {
        if (0 == std::memcmp(end, sub.Data(), sub.Size()))
        {
            return end - src.Data();
        }
        else
        {
            end -= stride;
        }
    }
    return std::string::npos;
}
std::vector<uint8_t> ByteReplace(const ByteBufferView& src, const ByteBufferView& sub, const ByteBufferView& replacement, size_t stride)
{
    std::vector<uint8_t> result;
    if (!src || !sub || src.Size() < sub.Size())
    {
        result.resize(src.Size());
        std::memcpy(result.data(), src.Data(), src.Size());
        return result;
    }
    std::vector<size_t> subpos;
    size_t              findOffset = 0;
    size_t              offset     = 0;
    while (findOffset = ByteFind(src, sub, offset, stride), findOffset != std::string::npos)
    {
        offset = findOffset + sub.Size();
        subpos.emplace_back(findOffset);
    }
    if (subpos.empty())
    {
        result.resize(src.Size());
        std::memcpy(result.data(), src.Data(), src.Size());
    }
    else
    {
        auto size = (replacement.Size() - sub.Size()) * subpos.size() + src.Size();
        result.resize(size);
        uint8_t* newData   = result.data();
        size_t   oldOffset = 0;
        for (auto pos : subpos)
        {
            std::memcpy(newData, src.Data() + oldOffset, pos - oldOffset);
            newData += pos - oldOffset;
            oldOffset = pos;
            std::memcpy(newData, replacement.Data(), replacement.Size());
            newData += replacement.Size();
            oldOffset += sub.Size();
        }
        std::memcpy(newData, src.Data() + oldOffset, src.Size() - oldOffset);
    }
    return result;
}
bool ByteEndWith(const ByteBufferView& src, const ByteBufferView& end)
{
    if (!src)
    {
        return false;
    }
    if (!end)
    {
        return true;
    }
    if (end.Size() > src.Size())
    {
        return false;
    }
    return 0 == std::memcmp(src.Data() + src.Size() - end.Size(), end.Data(), end.Size());
}
bool ByteStartWith(const ByteBufferView& src, const ByteBufferView& start)
{
    if (!src)
    {
        return false;
    }
    if (!start)
    {
        return true;
    }
    if (start.Size() > src.Size())
    {
        return false;
    }
    return 0 == std::memcmp(src.Data(), start.Data(), start.Size());
}

namespace
{
uint8_t CharToByte(const char& hex)
{
    if ('0' <= hex && hex <= '9')
    {
        return (hex - '0');
    }
    else if ('a' <= hex && hex <= 'f')
    {
        return (hex - 'a' + 10);
    }
    else if ('A' <= hex && hex <= 'F')
    {
        return (hex - 'A' + 10);
    }
    else
    {
        return std::numeric_limits<uint8_t>::max();
    }
}
} // namespace

std::optional<std::vector<uint8_t>> HexStringToBytes(const std::string& hex)
{
    if (0 != hex.size() % 2)
    {
        return std::nullopt;
    }
    std::vector<uint8_t> result(hex.size() / 2);
    uint8_t*             data = result.data();
    for (auto i = 0U; i < result.size(); i++)
    {
        auto bigByte   = CharToByte(hex.at(2 * i));
        auto smallByte = CharToByte(hex.at(2 * i + 1));
        if (std::numeric_limits<uint8_t>::max() == bigByte || std::numeric_limits<uint8_t>::max() == smallByte)
        {
            return std::nullopt;
        }
        *data = (bigByte << 4) + smallByte;
        ++data;
    }
    return std::move(result);
}
size_t CountLeftZero(uint32_t x)
{
    if (0 == x)
    {
        return 32;
    }
    int count = 0;
    while (!(x & 0x80000000))
    { ///0x80000000 = 1<<31
        count++;
        x <<= 1;
    }
    return count;
}
size_t CountLeftOne(uint32_t x)
{
    int count = 0;
    while ((x & 0x80000000))
    { ///0x80000000 = 1<<31
        count++;
        x <<= 1;
    }
    return count;
}
size_t CountRightZero(uint32_t x)
{
    if (0 == x)
    {
        return 32;
    }
    int count = 0;
    while (!(x & 0x00000001))
    {
        count++;
        x >>= 1;
    }
    return count;
}
size_t CountRightOne(uint32_t x)
{
    int count = 0;
    while ((x & 0x00000001))
    {
        count++;
        x >>= 1;
    }
    return count;
}

size_t CountLeftZero(uint16_t x)
{
    if (0 == x)
    {
        return 16;
    }
    int count = 0;
    while (!(x & 0x8000))
    {
        count++;
        x <<= 1;
    }
    return count;
}
size_t CountLeftOne(uint16_t x)
{
    int count = 0;
    while ((x & 0x8000))
    {
        count++;
        x <<= 1;
    }
    return count;
}
size_t CountRightZero(uint16_t x)
{
    if (0 == x)
    {
        return 16;
    }
    int count = 0;
    while (!(x & 0x0001))
    {
        count++;
        x >>= 1;
    }
    return count;
}
size_t CountRightOne(uint16_t x)
{
    int count = 0;
    while ((x & 0x0001))
    {
        count++;
        x >>= 1;
    }
    return count;
}

size_t CountLeftZero(uint8_t x)
{
    if (0 == x)
    {
        return 8;
    }
    int count = 0;
    while (!(x & 0x80))
    {
        count++;
        x <<= 1;
    }
    return count;
}
size_t CountLeftOne(uint8_t x)
{
    int count = 0;
    while ((x & 0x80))
    {
        count++;
        x <<= 1;
    }
    return count;
}
size_t CountRightZero(uint8_t x)
{
    if (0 == x)
    {
        return 8;
    }
    int count = 0;
    while (!(x & 0x01))
    {
        count++;
        x >>= 1;
    }
    return count;
}
size_t CountRightOne(uint8_t x)
{
    int count = 0;
    while ((x & 0x01))
    {
        count++;
        x >>= 1;
    }
    return count;
}
size_t CountZero(uint8_t x)
{
    return 8 - CountOne(x);
}
size_t CountOne(uint8_t x)
{
    int count = 0;
    while (x)
    {
        if (x & 0x01)
        {
            count++;
        }
        x >>= 1;
    }
    return count;
}

size_t CountZero(uint16_t x)
{
    return 16 - CountOne(x);
}
size_t CountOne(uint16_t x)
{
    int count = 0;
    while (x)
    {
        if (x & 0x0001)
        {
            count++;
        }
        x >>= 1;
    }
    return count;
}
size_t CountZero(uint32_t x)
{
    return 32 - CountOne(x);
}
size_t CountOne(uint32_t x)
{
    int count = 0;
    while (x)
    {
        if (x & 0x00000001)
        {
            count++;
        }
        x >>= 1;
    }
    return count;
}

size_t CountZero(const ByteBufferView& src)
{
    size_t count = 0;
    for (size_t index = 0; index < src.Size(); index++)
    {
        count += CountZero(src[index]);
    }
    return count;
}
size_t CountOne(const ByteBufferView& src)
{
    size_t count = 0;
    for (size_t index = 0; index < src.Size(); index++)
    {
        count += CountOne(src[index]);
    }
    return count;
}
} // namespace zeus

  // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic) 没有c++20，没有std::span可用