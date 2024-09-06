
#ifdef _WIN32
#include <string>
#include <Windows.h>
namespace zeus::Hardware
{

std::string GetSystemSMBIOSTable()
{
    std::string buffer;
    const auto  size = GetSystemFirmwareTable('RSMB', 0, 0, 0);
    buffer.resize(size);
    GetSystemFirmwareTable('RSMB', 0, buffer.data(), size);
    return buffer;
}

} // namespace zeus::Hardware
#endif