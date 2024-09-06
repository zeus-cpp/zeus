#pragma once
#include <string>

namespace zeus::Hardware
{

struct MonitorImpl
{
    std::string id;
    std::string devicePath;
    std::string model;
    std::string manufacturer;
    std::string productCode;
    std::string serial;
    size_t      physicalWidth  = 0; // mm
    size_t      physicalHeight = 0; // mm
};

struct EDIDInfo
{
    std::string model;
    std::string productCode;
    std::string manufacturer;
    std::string serial;
    size_t      physicalWidth  = 0; // mm
    size_t      physicalHeight = 0; // mm
};
} // namespace zeus::Hardware