#ifdef __linux__
#include <string>
#include "zeus/foundation/file/file_utils.h"

namespace fs = std::filesystem;

namespace zeus::Hardware
{
std::string GetSystemSMBIOSTable()
{
    return FileContent(fs::u8path("/sys/firmware/dmi/tables/DMI"), true).value_or("");
}

} // namespace zeus::Hardware
#endif
