#pragma once

#include <vector>
#include <string>
#include <memory>

namespace zeus::Hardware
{
struct MonitorImpl;
class Monitor
{
public:
    Monitor();
    ~Monitor();
    Monitor(const Monitor& other) noexcept;
    Monitor(Monitor&& other) noexcept;
    Monitor& operator=(const Monitor& other) noexcept;
    Monitor& operator=(Monitor&& other) noexcept;

    std::string Id() const noexcept;
    std::string DevicePath() const noexcept;
    std::string Model() const noexcept;
    std::string Manufacturer() const noexcept;
    std::string ProductCode() const noexcept;
    std::string SerialNumber() const noexcept;
    size_t      PhysicalWidth() const noexcept;  //mm
    size_t      PhysicalHeight() const noexcept; //mm
public:
    static std::vector<Monitor> ListAll();

private:
    std::unique_ptr<MonitorImpl> _impl;
};

} // namespace zeus::Hardware

#include "zeus/foundation/core/zeus_compatible.h"
