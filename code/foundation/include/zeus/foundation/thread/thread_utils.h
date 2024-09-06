#pragma once
#include <string>
#include <cstdint>

namespace zeus
{
void     SetThreadName(const std::string& threadName);
uint64_t GetThreadId();
}

#include "zeus/foundation/core/zeus_compatible.h"
