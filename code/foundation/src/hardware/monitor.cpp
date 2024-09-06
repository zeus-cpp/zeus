#include "zeus/foundation/hardware/monitor.h"
#include <vector>
#include <string>
#include <cstring>
#include "zeus/foundation/byte/byte_order.h"
#include "zeus/foundation/string/string_utils.h"
#include "impl/monitor_impl.h"

namespace zeus::Hardware
{

EDIDInfo ParseEDID(const std::vector<uint8_t>& edid)
{
    // https://zh.wikipedia.org/wiki/EDID
    // https://en.wikipedia.org/wiki/Extended_Display_Identification_Data
    constexpr size_t  kManufacturerOffset               = 8;
    constexpr size_t  kManufacturerLength               = 2;
    constexpr size_t  kProductIdOffset                  = 10;
    constexpr size_t  kProductIdLength                  = 2;
    constexpr size_t  kEDIDMaxHorizontalImageSizeOffset = 21;
    constexpr size_t  kEDIDMaxVerticalImageSizeOffset   = 22;
    constexpr size_t  kDescriptorOffset                 = 54;
    constexpr size_t  kNumDescriptors                   = 4;
    constexpr size_t  kDescriptorDataOffset             = 5;
    constexpr size_t  kDescriptorLength                 = 18;
    // The specifier types.
    constexpr uint8_t kMonitorNameDescriptor            = 0xfc;
    constexpr uint8_t kMonitorSerialNumberDescriptor    = 0xff;
    EDIDInfo          info                              = {};
    do
    {
        if (edid.size() < kManufacturerOffset + kManufacturerLength)
        {
            break;
        }
        {
            uint16_t manufacturerData = 0;
            std::memcpy(&manufacturerData, &edid.at(kManufacturerOffset), sizeof(manufacturerData));
            manufacturerData = FlipBytes(manufacturerData);
            uint8_t letter   = (manufacturerData >> 10) & 0b00011111;
            info.manufacturer.push_back(letter - 1 + 'A');
            letter = (manufacturerData >> 5) & 0b00011111;
            info.manufacturer.push_back(letter - 1 + 'A');
            letter = manufacturerData & 0b00011111;
            info.manufacturer.push_back(letter - 1 + 'A');
        }
        if (edid.size() < kProductIdOffset + kProductIdLength)
        {
            break;
        }
        info.productCode =
            zeus::BytesToHexString(&edid.at(kProductIdOffset + 1), 1, true) + zeus::BytesToHexString(&edid.at(kProductIdOffset), 1, true);
        if (edid.size() < kEDIDMaxVerticalImageSizeOffset + 1)
        {
            break;
        }
        info.physicalWidth  = edid.at(kEDIDMaxHorizontalImageSizeOffset) * 10;
        info.physicalHeight = edid.at(kEDIDMaxVerticalImageSizeOffset) * 10;
        for (size_t i = 0; i < kNumDescriptors; ++i)
        {
            if (edid.size() < kDescriptorOffset + (i + 1) * kDescriptorLength)
            {
                break;
            }
            const size_t offset = kDescriptorOffset + i * kDescriptorLength;

            if (0 == edid.at(offset) && 0 == edid.at(offset + 1) && 0 == edid.at(offset + 2) && kMonitorNameDescriptor == edid.at(offset + 3) &&
                0 == edid.at(offset + 4))
            {
                std::string name(reinterpret_cast<const char*>(&edid.at(offset + kDescriptorDataOffset)), kDescriptorLength - kDescriptorDataOffset);
                info.model = zeus::Trim(name);
                continue;
            }
            if (0 == edid.at(offset) && 0 == edid.at(offset + 1) && 0 == edid.at(offset + 2) &&
                kMonitorSerialNumberDescriptor == edid.at(offset + 3) && 0 == edid.at(offset + 4))
            {
                std::string serial(
                    reinterpret_cast<const char*>(&edid.at(offset + kDescriptorDataOffset)), kDescriptorLength - kDescriptorDataOffset
                );
                info.serial = zeus::Trim(serial);
                continue;
            }
        }
    }
    while (false);
    return info;
}

void FillMonitorImpl(MonitorImpl& monitorImpl, const EDIDInfo& EDIDInfo)
{
    monitorImpl.model          = EDIDInfo.model;
    monitorImpl.manufacturer   = EDIDInfo.manufacturer;
    monitorImpl.productCode    = EDIDInfo.productCode;
    monitorImpl.serial         = EDIDInfo.serial;
    monitorImpl.physicalWidth  = EDIDInfo.physicalWidth;
    monitorImpl.physicalHeight = EDIDInfo.physicalHeight;
}

Monitor::Monitor() : _impl(std::make_unique<MonitorImpl>())
{
}
Monitor::~Monitor()
{
}
Monitor::Monitor(const Monitor& other) noexcept : _impl(std::make_unique<MonitorImpl>(*other._impl))
{
}
Monitor::Monitor(Monitor&& other) noexcept : _impl(std::move(other._impl))
{
}
Monitor& Monitor::operator=(const Monitor& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
Monitor& Monitor::operator=(Monitor&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
std::string Monitor::Id() const noexcept
{
    return _impl->id;
}
std::string Monitor::DevicePath() const noexcept
{
    return _impl->devicePath;
}
std::string Monitor::Model() const noexcept
{
    return _impl->model;
}
std::string Monitor::Manufacturer() const noexcept
{
    return _impl->manufacturer;
}
std::string Monitor::ProductCode() const noexcept
{
    return _impl->productCode;
}
std::string Monitor::SerialNumber() const noexcept
{
    return _impl->serial;
}
size_t Monitor::PhysicalWidth() const noexcept
{
    return _impl->physicalWidth;
}
size_t Monitor::PhysicalHeight() const noexcept
{
    return _impl->physicalHeight;
}

} // namespace zeus::Hardware
