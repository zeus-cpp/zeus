#include "zeus/foundation/string/unit_conversion.h"
#include <sstream>

namespace
{
constexpr uint64_t KUnit = 1024;
constexpr uint64_t MUnit = KUnit * 1024;
constexpr uint64_t GUnit = MUnit * 1024;
constexpr uint64_t TUnit = GUnit * 1024;
constexpr uint64_t PUnit = TUnit * 1024;
constexpr uint64_t EUnit = PUnit * 1024;

constexpr uint64_t NetKUnit = 1000;
constexpr uint64_t NetMUnit = NetKUnit * 1000;
constexpr uint64_t NetGUnit = NetMUnit * 1000;
constexpr uint64_t NetTUnit = NetGUnit * 1000;
constexpr uint64_t NetPUnit = NetTUnit * 1000;
constexpr uint64_t NetEUnit = NetPUnit * 1000;
}
namespace zeus
{
std::string UnitConversion::Capacity::ToCapacityString(bool upper) const
{
    std::stringstream stream;
    stream.imbue(std::locale::classic());
    do
    {
        if (eUnit)
        {
            stream << rawUnit / (1.0 * EUnit) << (upper ? "EB" : "eb");
            break;
        }
        if (pUnit)
        {
            stream << rawUnit / (1.0 * PUnit) << (upper ? "PB" : "pb");
            break;
        }
        if (tUnit)
        {
            stream << rawUnit / (1.0 * TUnit) << (upper ? "TB" : "tb");
            break;
        }
        if (gUnit)
        {
            stream << rawUnit / (1.0 * GUnit) << (upper ? "GB" : "gb");
            break;
        }
        if (mUnit)
        {
            stream << rawUnit / (1.0 * MUnit) << (upper ? "MB" : "mb");
            break;
        }
        if (kUnit)
        {
            stream << rawUnit / (1.0 * KUnit) << (upper ? "KB" : "kb");
            break;
        }
        if (unit)
        {
            stream << rawUnit << (upper ? "BYTE" : "byte");
            break;
        }
        stream << 0 << (upper ? "BYTE" : "byte");
    }
    while (false);

    return stream.str();
}

std::string UnitConversion::Capacity::ToBitrateString(bool upper) const
{
    std::stringstream stream;
    stream.imbue(std::locale::classic());
    do
    {
        if (eUnit)
        {
            stream << rawUnit / (1.0 * NetEUnit) << (upper ? "Ebps" : "ebps");
            break;
        }
        if (pUnit)
        {
            stream << rawUnit / (1.0 * NetPUnit) << (upper ? "Pbps" : "pbps");
            break;
        }
        if (tUnit)
        {
            stream << rawUnit / (1.0 * NetTUnit) << (upper ? "Tbps" : "tbps");
            break;
        }
        if (gUnit)
        {
            stream << rawUnit / (1.0 * NetGUnit) << (upper ? "Gbps" : "gbps");
            break;
        }
        if (mUnit)
        {
            stream << rawUnit / (1.0 * NetMUnit) << (upper ? "Mbps" : "mbps");
            break;
        }
        if (kUnit)
        {
            stream << rawUnit / (1.0 * NetKUnit) << (upper ? "Kbps" : "kbps");
            break;
        }
        if (unit)
        {
            stream << rawUnit << (upper ? "bps" : "bps");
            break;
        }
        stream << 0 << (upper ? "bps" : "bps");
    }
    while (false);

    return stream.str();
}

UnitConversion::Capacity UnitConversion::CapacityUnitCast(uint64_t value)
{
    Capacity result;
    result.rawUnit = value;

    result.gUnit = value / EUnit;
    value %= EUnit;
    result.gUnit = value / PUnit;
    value %= PUnit;
    result.gUnit = value / TUnit;
    value %= TUnit;
    result.gUnit = value / GUnit;
    value %= GUnit;
    result.mUnit = value / MUnit;
    value %= MUnit;
    result.kUnit = value / KUnit;
    value %= KUnit;
    result.unit = value;
    return result;
}

UnitConversion::Capacity UnitConversion::CapacityKUnitCast(uint64_t value)
{
    return CapacityUnitCast(value * KUnit);
}

UnitConversion::Capacity UnitConversion::CapacityMUnitCast(uint64_t value)
{
    return CapacityUnitCast(value * MUnit);
}

UnitConversion::Capacity UnitConversion::CapacityGUnitCast(uint64_t value)
{
    return CapacityUnitCast(value * GUnit);
}

UnitConversion::Capacity UnitConversion::CapacityTUnitCast(uint64_t value)
{
    return CapacityUnitCast(value * TUnit);
}

UnitConversion::Capacity UnitConversion::CapacityPUnitCast(uint64_t value)
{
    return CapacityUnitCast(value * PUnit);
}

UnitConversion::Capacity UnitConversion::CapacityEUnitCast(uint64_t value)
{
    return CapacityUnitCast(value * EUnit);
}

UnitConversion::Capacity UnitConversion::BitrateUnitCast(uint64_t value)
{
    Capacity result;
    result.rawUnit = value;

    result.gUnit = value / NetEUnit;
    value %= NetEUnit;
    result.gUnit = value / NetPUnit;
    value %= NetPUnit;
    result.gUnit = value / NetTUnit;
    value %= NetTUnit;
    result.gUnit = value / NetGUnit;
    value %= NetGUnit;
    result.mUnit = value / NetMUnit;
    value %= NetMUnit;
    result.kUnit = value / NetKUnit;
    value %= NetKUnit;
    result.unit = value;
    return result;
}
UnitConversion::Capacity UnitConversion::BitrateKUnitCast(uint64_t value)
{
    return BitrateUnitCast(value * KUnit);
}
UnitConversion::Capacity UnitConversion::BitrateMUnitCast(uint64_t value)
{
    return BitrateUnitCast(value * MUnit);
}
UnitConversion::Capacity UnitConversion::BitrateGUnitCast(uint64_t value)
{
    return BitrateUnitCast(value * GUnit);
}
UnitConversion::Capacity UnitConversion::BitrateTUnitCast(uint64_t value)
{
    return BitrateUnitCast(value * TUnit);
}
UnitConversion::Capacity UnitConversion::BitratePUnitCast(uint64_t value)
{
    return BitrateUnitCast(value * PUnit);
}
UnitConversion::Capacity UnitConversion::BitrateEUnitCast(uint64_t value)
{
    return BitrateUnitCast(value * EUnit);
}
} // namespace zeus