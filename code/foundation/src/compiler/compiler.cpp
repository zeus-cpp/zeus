#include "zeus/foundation/compiler/compiler.h"
namespace zeus
{

std::string GetClassName(const std::type_info& type)
{
    auto fullName = GetFullClassName(type);

    auto pos = fullName.rfind("::");
    if (pos != std::string::npos)
    {
        return fullName.substr(pos + 2);
    }
    return fullName;
}

} // namespace zeus
