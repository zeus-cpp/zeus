#pragma once
#include <string>
#include <typeinfo>

namespace zeus
{
std::string GetFullClassName(const std::type_info& type);
std::string GetClassName(const std::type_info& type);
}
#include "zeus/foundation/core/zeus_compatible.h"
