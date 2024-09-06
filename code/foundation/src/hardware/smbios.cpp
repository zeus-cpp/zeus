#include "zeus/foundation/hardware/smbios.h"
#include <cstring>

namespace zeus::Hardware
{
struct SMBIOSImpl
{
    SMBIOS::InfoType         type   = SMBIOS::InfoType::kSMBIOS_END;
    uint8_t                  length = 0;
    uint16_t                 handle = 0;
    std::vector<uint8_t>     data;
    std::vector<std::string> stringTable;
};

std::string GetSystemSMBIOSTable();

SMBIOS::SMBIOS() : _impl(std::make_unique<SMBIOSImpl>())
{
}

SMBIOS::~SMBIOS()
{
}

SMBIOS::SMBIOS(const SMBIOS& other) : _impl(std::make_unique<SMBIOSImpl>(*other._impl))
{
}

SMBIOS::SMBIOS(SMBIOS&& other) noexcept : _impl(std::move(other._impl))
{
}

SMBIOS& SMBIOS::operator=(const SMBIOS& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

SMBIOS& SMBIOS::operator=(SMBIOS&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

SMBIOS::InfoType SMBIOS::Type() const noexcept
{
    return _impl->type;
}

uint8_t SMBIOS::Length() const noexcept
{
    return _impl->length;
}

uint16_t SMBIOS::Handle() const noexcept
{
    return _impl->handle;
}

const std::vector<uint8_t>& SMBIOS::Data() const noexcept
{
    return _impl->data;
}

const std::vector<std::string>& SMBIOS::StringTable() const noexcept
{
    return _impl->stringTable;
}

std::vector<SMBIOS> SMBIOS::GetSMBIOS(InfoType type)
{
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic) 没有c++20，没有std::span可用
#pragma pack(push)
#pragma pack(1)
    struct SMBIOSHead
    {
        uint8_t  type   = 0;
        uint8_t  length = 0;
        uint16_t handle = 0;
    };
#pragma pack(pop)
    std::vector<SMBIOS> result;
    auto                data = GetSystemSMBIOSTable();
    if (!data.empty())
    {
        const auto* tableHead = reinterpret_cast<const SMBIOSHead*>(data.data());
        const auto* tableEnd  = reinterpret_cast<const SMBIOSHead*>(reinterpret_cast<const uint8_t*>(data.data()) + data.size());
        while (tableHead < tableEnd && tableHead->type != static_cast<uint8_t>(SMBIOS::InfoType::kSMBIOS_END))
        {
            const auto* begin = reinterpret_cast<const uint8_t*>(tableHead);
            SMBIOS      item;
            bool        match = false;
            if (tableHead->type == static_cast<uint8_t>(type) && tableHead->length > sizeof(SMBIOSHead))
            {
                match                 = true;
                item._impl->type      = static_cast<SMBIOS::InfoType>(tableHead->type);
                item._impl->length    = tableHead->length;
                item._impl->handle    = tableHead->handle;
                const auto dataLength = tableHead->length - sizeof(SMBIOSHead);
                item._impl->data.resize(dataLength);
                std::memcpy(item._impl->data.data(), begin + sizeof(SMBIOSHead), dataLength);
            }
            size_t stringBegin = tableHead->length;
            size_t stringEnd   = stringBegin;
            do //NOLINT(cppcoreguidelines-avoid-do-while) 这里的逻辑就是do-while
            {
                while (0 != begin[stringEnd])
                {
                    stringEnd++;
                }
                if (match && stringBegin != stringEnd)
                {
                    item._impl->stringTable.emplace_back(begin + stringBegin, begin + stringEnd);
                }
                stringBegin = stringEnd + 1;
                stringEnd   = stringBegin;
            }
            while (0 != (begin[stringEnd]));
            if (match)
            {
                result.emplace_back(std::move(item));
            }
            tableHead = reinterpret_cast<const SMBIOSHead*>(begin + stringBegin + 1);
        }
    }
    return result;
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic) 没有c++20，没有std::span可用
}
} // namespace zeus::Hardware