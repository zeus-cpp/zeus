#include "zeus/foundation/hardware/memory.h"
#include <sstream>
#include "zeus/foundation/hardware/smbios.h"
#include "zeus/foundation/string/unit_conversion.h"

namespace zeus::Hardware
{

namespace
{

std::string FindMemoryName(const std::string& manufacturer, const std::string& /*product*/, uint32_t speed, uint64_t size, MemoryBank::BankType type)
{
    std::stringstream stream;
    stream.imbue(std::locale::classic());
    if (!manufacturer.empty())
    {
        stream << manufacturer << " ";
    }
    else
    {
        stream << "Unknown ";
    }
    stream << MemoryBank::TypeName(type) << " ";
    stream << speed << "MHZ ";
    stream << zeus::UnitConversion::CapacityKUnitCast(size).ToCapacityString();
    return stream.str();
}

}
struct MemoryBankImpl
{
    std::string          name;
    std::string          manufacturer;
    std::string          serialNumber;
    std::string          partNumber;
    uint64_t             capacity   = 0; //KB
    uint32_t             colokSpeed = 0; //Mhz
    uint32_t             dataWidth  = 0; //bit
    MemoryBank::BankType type       = MemoryBank::BankType::kUnknown;
};
MemoryBank::MemoryBank() : _impl(std::make_unique<MemoryBankImpl>())
{
}
MemoryBank::~MemoryBank()
{
}
MemoryBank::MemoryBank(const MemoryBank& other) : _impl(std::make_unique<MemoryBankImpl>(*other._impl))
{
}
MemoryBank::MemoryBank(MemoryBank&& other) noexcept : _impl(std::move(other._impl))
{
}
MemoryBank& MemoryBank::operator=(const MemoryBank& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
MemoryBank& MemoryBank::operator=(MemoryBank&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string MemoryBank::Name() const
{
    return _impl->name;
}
std::string MemoryBank::Manufacturer() const
{
    return _impl->manufacturer;
}
std::string MemoryBank::SerialNumber() const
{
    return _impl->serialNumber;
}
std::string MemoryBank::PartNumber() const
{
    return _impl->partNumber;
}
uint64_t MemoryBank::Capacity() const
{
    return _impl->capacity;
}
uint32_t MemoryBank::ColokSpeed() const
{
    return _impl->colokSpeed;
}
uint32_t MemoryBank::DataWidth() const
{
    return _impl->dataWidth;
}

MemoryBank::BankType MemoryBank::Type() const
{
    return _impl->type;
}

std::vector<MemoryBank> MemoryBank::GetMemoryBankList()
{
#pragma pack(push)
#pragma pack(1)
    struct MemoryDevice
    {
        uint16_t PhysicalArrayHandle;
        uint16_t ErrorInformationHandle;
        uint16_t TotalWidth;
        uint16_t DataWidth;
        uint16_t Size;
        uint8_t  FormFactor;
        uint8_t  DeviceSet;
        uint8_t  DeviceLocator;
        uint8_t  BankLocator;
        uint8_t  MemoryType;
        uint16_t TypeDetail;
        uint16_t Speed;
        uint8_t  Manufacturer;
        uint8_t  SN;
        uint8_t  AssetTag;
        uint8_t  PartNumber;
        uint8_t  Attributes;
        uint32_t ExtendedSize;
    };
#pragma pack(pop)
    std::vector<MemoryBank> result;
    const auto              data = SMBIOS::GetSMBIOS(zeus::Hardware::SMBIOS::InfoType::kSMBIOS_MEMORYDEVICE);
    for (auto& item : data)
    {
        const auto* info = reinterpret_cast<const MemoryDevice*>(item.Data().data());
        if (info->Size)
        {
            MemoryBank bankInfo;
            if (0x7FFF == info->Size)
            {
                bankInfo._impl->capacity = static_cast<uint64_t>(info->ExtendedSize) * 1024;
            }
            else
            {
                if (0x8000 & info->Size)
                {
                    bankInfo._impl->capacity = info->Size;
                }
                else
                {
                    bankInfo._impl->capacity = info->Size * 1024;
                }
            }
            bankInfo._impl->dataWidth  = info->DataWidth;
            bankInfo._impl->colokSpeed = info->Speed;
            if (info->SN && item.StringTable().size() >= info->SN)
            {
                bankInfo._impl->serialNumber = item.StringTable().at(info->SN - 1);
            }
            if (info->PartNumber && item.StringTable().size() >= info->PartNumber)
            {
                bankInfo._impl->partNumber = item.StringTable().at(info->PartNumber - 1);
            }
            if (info->Manufacturer && item.StringTable().size() >= info->Manufacturer)
            {
                bankInfo._impl->manufacturer = item.StringTable().at(info->Manufacturer - 1);
            }

            bankInfo._impl->type = static_cast<MemoryBank::BankType>(info->MemoryType);
            bankInfo._impl->name = FindMemoryName(
                bankInfo._impl->manufacturer, bankInfo._impl->partNumber, bankInfo._impl->colokSpeed, bankInfo._impl->capacity, bankInfo._impl->type
            );

            result.emplace_back(std::move(bankInfo));
        }
    }
    return result;
}
std::string MemoryBank::TypeName(BankType type)
{
    switch (type)
    {
    case BankType::kOther:
        return "Other";
    case BankType::kUnknown:
        return "Unknown";
    case BankType::kDRAM:
        return "DRAM";
    case BankType::kEDRAM:
        return "EDRAM";
    case BankType::kVRAM:
        return "VRAM";
    case BankType::kSRAM:
        return "SRAM";
    case BankType::kRAM:
        return "RAM";
    case BankType::kROM:
        return "ROM";
    case BankType::kFLASH:
        return "FLASH";
    case BankType::kEEPROM:
        return "EEPROM";
    case BankType::kFEPROM:
        return "FEPROM";
    case BankType::kEPROM:
        return "EPROM";
    case BankType::kCDRAM:
        return "CDRAM";
    case BankType::k3DRAM:
        return "3DRAM";
    case BankType::kSDRAM:
        return "SDRAM";
    case BankType::kSGRAM:
        return "SGRAM";
    case BankType::kRDRAM:
        return "RDRAM";
    case BankType::kDDR:
        return "DDR";
    case BankType::kDDR2:
        return "DDR2";
    case BankType::kDDR2_FB:
        return "DDR2_FB";
    case BankType::kDDR3:
        return "DDR3";
    case BankType::kFBD2:
        return "FBD2";
    case BankType::kDDR4:
        return "DDR4";
    case BankType::kLPDDR:
        return "LPDDR";
    case BankType::kLPDDR2:
        return "LPDDR2";
    case BankType::kLPDDR3:
        return "LPDDR3";
    case BankType::kLPDDR4:
        return "LPDDR4";
    case BankType::kHBM:
        return "HBM";
    case BankType::kHBM2:
        return "HBM2";
    case BankType::kDDR5:
        return "DDR5";
    case BankType::kLPDDR5:
        return "LPDDR5";
    default:
        return "Unknown";
    }
}
} // namespace zeus::Hardware