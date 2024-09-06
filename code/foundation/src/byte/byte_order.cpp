#include "zeus/foundation/byte/byte_order.h"

uint16_t zeus::FlipBytes(const uint16_t& value)
{
    return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
}

uint32_t zeus::FlipBytes(const uint32_t& value)
{
    return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
}

uint64_t zeus::FlipBytes(const uint64_t& value)
{
    uint32_t hi = uint32_t(value >> 32);
    uint32_t lo = uint32_t(value & 0xFFFFFFFF);
    return uint64_t(FlipBytes(hi)) | (uint64_t(FlipBytes(lo)) << 32);
}

int16_t zeus::FlipBytes(const int16_t& value)
{
    return FlipBytes(uint16_t(value));
}

int32_t zeus::FlipBytes(const int32_t& value)
{
    return FlipBytes(uint32_t(value));
}

int64_t zeus::FlipBytes(const int64_t& value)
{
    return FlipBytes(uint64_t(value));
}

uint16_t zeus::FlipBytes(const uint16_t* value)
{
    return FlipBytes(*value);
}

uint32_t zeus::FlipBytes(const uint32_t* value)
{
    return FlipBytes(*value);
}

uint64_t zeus::FlipBytes(const uint64_t* value)
{
    return FlipBytes(*value);
}

int16_t zeus::FlipBytes(const int16_t* value)
{
    return FlipBytes(*value);
}

int32_t zeus::FlipBytes(const int32_t* value)
{
    return FlipBytes(*value);
}

int64_t zeus::FlipBytes(const int64_t* value)
{
    return FlipBytes(*value);
}
