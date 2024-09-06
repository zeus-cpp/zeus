#pragma once

#include <vector>
#include <string>
namespace zeus::Hardware
{
class Screen
{
public:
    struct ScreenInfo
    {
        std::string name;
        size_t      currentWidth  = 0; // px
        size_t      currentHeight = 0; // px
        bool        primary       = false;
        int32_t     left          = 0;
        int32_t     top           = 0;
        size_t      frequency     = 0;
    };
    //只能获取到当前session的屏幕信息
    static std::vector<ScreenInfo> GetDisplayScreenInfo();
};
} // namespace zeus::Hardware

#include "zeus/foundation/core/zeus_compatible.h"
