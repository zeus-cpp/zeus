#include "zeus/foundation/hardware/system.h"
#include "zeus/foundation/hardware/smbios.h"
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus::Hardware
{

struct SystemImpl
{
    std::string manufacturer;
    std::string product;
    std::string serialNumber;
};

System::System() : _impl(std::make_unique<SystemImpl>())
{
}

System::~System()
{
}

System::System(const System& other) noexcept : _impl(std::make_unique<SystemImpl>(*other._impl))
{
}

System::System(System&& other) noexcept : _impl(std::move(other._impl))
{
}

System& System::operator=(const System& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

System& System::operator=(System&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

std::string System::Manufacturer() const noexcept
{
    return _impl->manufacturer;
}

std::string System::Product() const noexcept
{
    return _impl->product;
}

std::string System::SerialNumber() const noexcept
{
    return _impl->serialNumber;
}

System System::GetSystem()
{
#pragma pack(push)
#pragma pack(1)
    struct BiosSystemInfo
    {
        uint8_t Manufacturer;
        uint8_t Product;
        uint8_t Version;
        uint8_t SN;
        uint8_t UUID[16];
        uint8_t WakeUpType;
        uint8_t SKUNumber;
        uint8_t Family;
    };
#pragma pack(pop)
    System     result;
    const auto data = SMBIOS::GetSMBIOS(zeus::Hardware::SMBIOS::InfoType::kSMBIOS_SYSTEM);
    if (!data.empty())
    {
        const auto* info = reinterpret_cast<const BiosSystemInfo*>(data.front().Data().data());
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
    if (result._impl->product.empty())
    {
        result._impl->product = Trim(FileContent(std::filesystem::path("/sys/class/dmi/id/product_name"), false).value_or(""));
    }
    if (result._impl->manufacturer.empty())
    {
        result._impl->manufacturer = Trim(FileContent(std::filesystem::path("/sys/class/dmi/id/sys_vendor"), false).value_or(""));
    }
    if (result._impl->serialNumber.empty())
    {
        result._impl->serialNumber = Trim(FileContent(std::filesystem::path("/sys/class/dmi/id/product_serial"), false).value_or(""));
    }
#endif
    return result;
}
} // namespace zeus::Hardware
