#pragma once

#include <string>
#include <memory>

namespace zeus
{
namespace Hardware
{
struct SystemImpl;
class System
{
public:
    System();
    ~System();
    System(const System& other) noexcept;
    System(System&& other) noexcept;
    System&     operator=(const System& other) noexcept;
    System&     operator=(System&& other) noexcept;
    std::string Manufacturer() const noexcept;
    std::string Product() const noexcept;
    std::string SerialNumber() const noexcept;
public:
    static System GetSystem();
private:
    std::unique_ptr<SystemImpl> _impl;
};
}
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
