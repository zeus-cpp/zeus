#include "zeus/foundation/system/shared_library.h"

namespace zeus
{
SharedLibrary::operator bool() const noexcept
{
    return !Empty();
}

bool SharedLibrary::HasSymbol(const std::string& name) const noexcept
{
    return GetSymbol(name) != nullptr;
}

} // namespace zeus