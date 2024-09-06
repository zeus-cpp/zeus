#pragma once

#include <cstdint>
#include <string>

namespace zeus
{

class UnitConversion
{
public:
    struct Capacity
    {
        size_t   unit    = 0;
        size_t   kUnit   = 0;
        size_t   mUnit   = 0;
        size_t   gUnit   = 0;
        size_t   tUnit   = 0;
        size_t   pUnit   = 0;
        size_t   eUnit   = 0;
        uint64_t rawUnit = 0;

        std::string ToCapacityString(bool upper = true) const;

        std::string ToBitrateString(bool upper = true) const;
    };

    static Capacity CapacityUnitCast(uint64_t value);

    static Capacity CapacityKUnitCast(uint64_t value);

    static Capacity CapacityMUnitCast(uint64_t value);

    static Capacity CapacityGUnitCast(uint64_t value);

    static Capacity CapacityTUnitCast(uint64_t value);

    static Capacity CapacityPUnitCast(uint64_t value);

    static Capacity CapacityEUnitCast(uint64_t value);

    static Capacity BitrateUnitCast(uint64_t value);

    static Capacity BitrateKUnitCast(uint64_t value);

    static Capacity BitrateMUnitCast(uint64_t value);

    static Capacity BitrateGUnitCast(uint64_t value);

    static Capacity BitrateTUnitCast(uint64_t value);

    static Capacity BitratePUnitCast(uint64_t value);

    static Capacity BitrateEUnitCast(uint64_t value);
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
