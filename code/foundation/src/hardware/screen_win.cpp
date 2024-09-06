#include "zeus/foundation/hardware/screen.h"
#ifdef _WIN32
#include <Windows.h>
#include "zeus/foundation/string/charset_utils.h"

namespace zeus::Hardware
{
std::vector<Screen::ScreenInfo> Screen::GetDisplayScreenInfo()
{
    std::vector<ScreenInfo> result;
    uint32_t                i       = 0;
    DISPLAY_DEVICEW         display = {};
    display.cb                      = sizeof(display);
    while (::EnumDisplayDevicesW(nullptr, i, &display, 0))
    {
        i++;
        if (display.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        {
            continue;
        }
        DEVMODEW deviceMode      = {};
        deviceMode.dmSize        = sizeof(DEVMODEW);
        deviceMode.dmDriverExtra = 0;
        if (EnumDisplaySettingsExW(display.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode, 0))
        {
            ScreenInfo info;
            info.name          = zeus::CharsetUtils::UnicodeToUTF8(display.DeviceName);
            info.currentWidth  = deviceMode.dmPelsWidth;
            info.currentHeight = deviceMode.dmPelsHeight;
            info.left          = deviceMode.dmPosition.x;
            info.top           = deviceMode.dmPosition.y;
            info.primary       = display.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE;
            info.frequency     = deviceMode.dmDisplayFrequency;
            result.emplace_back(info);
        }
    }
    return result;
}
} // namespace zeus::Hardware

#endif
