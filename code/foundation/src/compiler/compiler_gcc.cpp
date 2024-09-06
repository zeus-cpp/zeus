#include "zeus/foundation/compiler/compiler.h"
#ifdef __GNUC__
#include <cxxabi.h>

namespace zeus
{

std::string GetFullClassName(const std::type_info& type)
{
    std::string typeName = type.name();
    int         status;
    char*       realname = nullptr;
    realname             = abi::__cxa_demangle(typeName.c_str(), nullptr, nullptr, &status);
    if (0 == status)
    {
        typeName = realname;
        free(realname);
    }
    return typeName;
}
}
#endif