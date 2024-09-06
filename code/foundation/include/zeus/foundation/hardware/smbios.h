#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace zeus
{
namespace Hardware
{
struct SMBIOSImpl;
class SMBIOS
{
public:
    enum class InfoType : uint8_t
    {
        kSMBIOS_BIOS            = 0,
        kSMBIOS_SYSTEM          = 1,
        kSMBIOS_BASEBOARD       = 2,
        kSMBIOS_PROCESSOR       = 4,
        kSMBIOS_PROCESSOR_CACHE = 7,
        kSMBIOS_MEMORYDEVICE    = 17,
        kSMBIOS_END             = 127,
    };
    SMBIOS();
    ~SMBIOS();
    SMBIOS(const SMBIOS& other);
    SMBIOS(SMBIOS&& other) noexcept;
    SMBIOS& operator=(const SMBIOS& other);
    SMBIOS& operator=(SMBIOS&& other) noexcept;

    InfoType                        Type() const noexcept;
    uint8_t                         Length() const noexcept;
    uint16_t                        Handle() const noexcept;
    const std::vector<uint8_t>&     Data() const noexcept;
    const std::vector<std::string>& StringTable() const noexcept;
public:
    static std::vector<SMBIOS> GetSMBIOS(InfoType type);
private:
    std::unique_ptr<SMBIOSImpl> _impl;
};
} // namespace Hardware
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
