#include "zeus/foundation/core/system_error.h"

#ifdef __linux__

#include "zeus/foundation/core/posix/error.h"

namespace zeus
{

std::string SystemErrorMessage(int code)
{
    return ErrnoDescription(code);
}

} // namespace zeus

#endif