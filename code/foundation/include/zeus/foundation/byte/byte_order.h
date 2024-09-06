#pragma once
#include <stdint.h>

namespace zeus
{
uint16_t FlipBytes(const uint16_t& value);
uint32_t FlipBytes(const uint32_t& value);
uint64_t FlipBytes(const uint64_t& value);
int16_t FlipBytes(const int16_t& value);
int32_t FlipBytes(const int32_t& value);
int64_t FlipBytes(const int64_t& value);

uint16_t FlipBytes(const uint16_t* value);
uint32_t FlipBytes(const uint32_t* value);
uint64_t FlipBytes(const uint64_t* value);
int16_t FlipBytes(const int16_t* value);
int32_t FlipBytes(const int32_t* value);
int64_t FlipBytes(const int64_t* value);
}

#include "zeus/foundation/core/zeus_compatible.h"
