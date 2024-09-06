#pragma once

#ifdef __linux__
#include <string>

namespace zeus
{
std::string ErrnoName(int error);
std::string ErrnoDescription(int error);
}

#endif
#include "zeus/foundation/core/zeus_compatible.h"
