#include "zeus/foundation/hardware/base_board.h"
#include "zeus/foundation/hardware/smbios.h"
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus::Hardware
{
struct BaseBoardImpl
{
    std::string manufacturer;
    std::string product;
    std::string serialNumber;
};

BaseBoard::BaseBoard() : _impl(std::make_unique<BaseBoardImpl>())
{
}

BaseBoard::~BaseBoard()
{
}

BaseBoard::BaseBoard(const BaseBoard& other) noexcept : _impl(std::make_unique<BaseBoardImpl>(*other._impl))
{
}

BaseBoard::BaseBoard(BaseBoard&& other) noexcept : _impl(std::move(other._impl))
{
}

BaseBoard& BaseBoard::operator=(const BaseBoard& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

BaseBoard& BaseBoard::operator=(BaseBoard&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

std::string BaseBoard::Manufacturer() const noexcept
{
    return _impl->manufacturer;
}

std::string BaseBoard::Product() const noexcept
{
    return _impl->product;
}

std::string BaseBoard::SerialNumber() const noexcept
{
    return _impl->serialNumber;
}

BaseBoard BaseBoard::GetBaseBoard()
{
#pragma pack(push)
#pragma pack(1)
    struct BoardInfo
    {
        uint8_t   Manufacturer;
        uint8_t   Product;
        uint8_t   Version;
        uint8_t   SN;
        uint8_t   AssetTag;
        uint8_t   FeatureFlags;
        uint8_t   LocationInChassis;
        uint16_t  ChassisHandle;
        uint8_t   Type;
        uint8_t   NumObjHandle;
        uint16_t* pObjHandle;
    };
#pragma pack(pop)
    BaseBoard  result;
    const auto data = SMBIOS::GetSMBIOS(zeus::Hardware::SMBIOS::InfoType::kSMBIOS_BASEBOARD);
    if (!data.empty())
    {
        const auto* info = reinterpret_cast<const BoardInfo*>(data.front().Data().data());
        if (info->Manufacturer && data.front().StringTable().size() >= info->Manufacturer)
        {
            result._impl->manufacturer = data.front().StringTable().at(info->Manufacturer - 1);
        }
        if (info->Product && data.front().StringTable().size() >= info->Product)
        {
            result._impl->product = data.front().StringTable().at(info->Product - 1);
        }
        if (info->SN && data.front().StringTable().size() >= info->SN)
        {
            result._impl->serialNumber = data.front().StringTable().at(info->SN - 1);
        }
    }
#ifdef __linux__
    using namespace std::literals;
    if (result._impl->product.empty())
    {
        result._impl->product = Trim(FileContent("/sys/class/dmi/id/board_name"sv, false).value_or(""));
    }
    if (result._impl->manufacturer.empty())
    {
        result._impl->manufacturer = Trim(FileContent("/sys/class/dmi/id/board_vendor"sv, false).value_or(""));
    }
    if (result._impl->serialNumber.empty())
    {
        result._impl->serialNumber = Trim(FileContent("/sys/class/dmi/id/board_serial"sv, false).value_or(""));
    }
#endif
    return result;
}
} // namespace zeus::Hardware
