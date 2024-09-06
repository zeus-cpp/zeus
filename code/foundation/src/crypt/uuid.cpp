#include "zeus/foundation/crypt/uuid.h"
#include <sstream>
#include <iomanip>
#include <ios>
#include <array>
#include <cstring>

#ifdef _WIN32
#include <objbase.h>
#endif
#ifdef __linux__
#include <uuid/uuid.h>
#endif
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/byte/byte_order.h"
#include "zeus/foundation/byte/byte_utils.h"
namespace zeus
{

struct UuidImpl
{
    std::array<uint8_t, 16> data = {};
};

Uuid::Uuid() : _impl(std::make_unique<UuidImpl>())
{
}

Uuid::~Uuid()
{
}

Uuid::Uuid(const Uuid& other) : _impl(std::make_unique<UuidImpl>(*other._impl))
{
}

Uuid::Uuid(Uuid&& other) noexcept : _impl(std::move(other._impl))
{
}

Uuid& Uuid::operator=(const Uuid& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

Uuid& Uuid::operator=(Uuid&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

Uuid Uuid::GenerateRandom()
{
    Uuid result;
#ifdef _WIN32

    GUID newId;
    ::CoCreateGuid(&newId);

    result = FromGuid(newId);
#endif
#ifdef __linux__

    uuid_t id;
    uuid_generate(id);
    for (size_t index = 0; index < result._impl->data.size(); ++index)
    {
        result._impl->data[index] = id[index];
    }

#endif
    return result;
}

std::string Uuid::toString(const std::string& split, bool up) const
{
    std::stringstream str;
    str.imbue(std::locale::classic());
    if (up)
    {
        str.setf(std::ios_base::uppercase);
    }
    else
    {
        str.unsetf(std::ios_base::uppercase);
    }
    str << std::hex << std::setfill('0') << std::setw(2) << (int) _impl->data[0] << std::setw(2) << (int) _impl->data[1] << std::setw(2)
        << (int) _impl->data[2] << std::setw(2) << (int) _impl->data[3] << split << std::setw(2) << (int) _impl->data[4] << std::setw(2)
        << (int) _impl->data[5] << split << std::setw(2) << (int) _impl->data[6] << std::setw(2) << (int) _impl->data[7] << split << std::setw(2)
        << (int) _impl->data[8] << std::setw(2) << (int) _impl->data[9] << split << std::setw(2) << (int) _impl->data[10] << std::setw(2)
        << (int) _impl->data[11] << std::setw(2) << (int) _impl->data[12] << std::setw(2) << (int) _impl->data[13] << std::setw(2)
        << (int) _impl->data[14] << std::setw(2) << (int) _impl->data[15];
    return str.str();
}
#ifdef _WIN32
std::string Uuid::toWindowsString() const
{
    return "{" + toString("-") + "}";
}
std::string Uuid::toString(const GUID& guid)
{
    OLECHAR guidString[40] = {0};
    ::StringFromGUID2(guid, guidString, sizeof(guidString));
    return CharsetUtils::UnicodeToUTF8(guidString);
}

Uuid Uuid::FromGuid(const GUID& guid)
{
    Uuid result;
    *reinterpret_cast<uint32_t*>(&result._impl->data[0]) = FlipBytes(reinterpret_cast<const uint32_t*>(&guid.Data1));
    *reinterpret_cast<uint16_t*>(&result._impl->data[4]) = FlipBytes(reinterpret_cast<const uint16_t*>(&guid.Data2));
    *reinterpret_cast<uint16_t*>(&result._impl->data[6]) = FlipBytes(reinterpret_cast<const uint16_t*>(&guid.Data3));
    std::memcpy(&result._impl->data[8], guid.Data4, sizeof(guid.Data4));
    return result;
}
GUID Uuid::toGuid() const
{
    GUID result;
    result.Data1 = FlipBytes(reinterpret_cast<const uint32_t*>(&_impl->data[0]));
    result.Data2 = FlipBytes(reinterpret_cast<const uint16_t*>(&_impl->data[4]));
    result.Data3 = FlipBytes(reinterpret_cast<const uint16_t*>(&_impl->data[6]));
    std::memcpy(result.Data4, &_impl->data[8], sizeof(result.Data4));
    return result;
}
#endif

std::optional<Uuid> Uuid::FromWindowsString(const std::string& guid)
{
    if (38 != guid.size())
    {
        return std::nullopt;
    }
    if (guid.front() != '{' || guid.back() != '}')
    {
        return std::nullopt;
    }
    Uuid result;
    //第一段
    auto temp  = guid.substr(1, 8);
    auto bytes = HexStringToBytes(temp);
    if (!bytes || bytes->size() != 4)
    {
        return std::nullopt;
    }
    std::memcpy(&result._impl->data[0], bytes->data(), bytes->size());
    //第二段
    temp  = guid.substr(2 + 8, 4);
    bytes = HexStringToBytes(temp);
    if (!bytes->data() || bytes->size() != 2)
    {
        return std::nullopt;
    }
    std::memcpy(&result._impl->data[4], bytes->data(), bytes->size());
    //第三段
    temp  = guid.substr(3 + 12, 4);
    bytes = HexStringToBytes(temp);
    if (!bytes->data() || bytes->size() != 2)
    {
        return std::nullopt;
    }
    std::memcpy(&result._impl->data[6], bytes->data(), bytes->size());
    //第四段
    temp  = guid.substr(4 + 16, 4);
    bytes = HexStringToBytes(temp);
    if (!bytes->data() || bytes->size() != 2)
    {
        return std::nullopt;
    }
    std::memcpy(&result._impl->data[8], bytes->data(), bytes->size());
    //第五段
    temp  = guid.substr(5 + 20, 12);
    bytes = HexStringToBytes(temp);
    if (!bytes->data() || bytes->size() != 6)
    {
        return std::nullopt;
    }
    std::memcpy(&result._impl->data[10], bytes->data(), bytes->size());
    return result;
}

bool operator==(Uuid const& lhs, Uuid const& rhs) noexcept
{
    return lhs._impl->data == rhs._impl->data;
}

bool operator<(Uuid const& lhs, Uuid const& rhs) noexcept
{
    return lhs._impl->data < rhs._impl->data;
}
} // namespace zeus
