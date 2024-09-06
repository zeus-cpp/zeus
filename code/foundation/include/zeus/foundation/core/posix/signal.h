#pragma once

#ifdef __linux__
#include <string>

namespace zeus
{
std::string SignalName(int sig);
}

#endif
#include "zeus/foundation/core/zeus_compatible.h"
