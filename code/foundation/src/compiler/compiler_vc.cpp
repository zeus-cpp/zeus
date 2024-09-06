#include "zeus/foundation/compiler/compiler.h"
#ifdef _WIN32
#include <zeus/foundation/string/string_utils.h>
namespace zeus
{
std::string GetFullClassName(const std::type_info& type)
{
    static const std::string kClassPrefix  = "class ";
    static const std::string kStructPrefix = "struct ";
    std::string              typeName      = type.name();
    if (StartWith(typeName, kClassPrefix))
    {
        typeName = typeName.substr(kClassPrefix.size());
    }
    else if (StartWith(typeName, kStructPrefix))
    {
        typeName = typeName.substr(kStructPrefix.size());
    }
    return typeName;
}

} // namespace zeus
#endif